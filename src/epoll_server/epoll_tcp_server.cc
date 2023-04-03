#include "epoll_server/epoll_tcp_server.h"

namespace ftcp {

EpollTCPServer::EpollTCPServer(const EpollRunMode& mode, const int& parallel, const std::string& listen_addr, const int& listen_port) : mode_(mode), parallel_num_(parallel), listen_addr_(listen_addr), listen_port_(listen_port) {
  stop_ = false;
}

ReturnCode EpollTCPServer::Init() {
  ReturnCode ret = ReturnCode::SUCCESS;
  main_ep_fd_ = epoll_create1(0);
  ret = InitListenSocket();
  return ret;
}

void EpollTCPServer::Start() {
  if (mode_ == EpollRunMode::UseProcess) {
    CreateProcesses();
  } else if (mode_ == EpollRunMode::UseThread) {
    CreateThreads();
  }
  StartMainEpoll();
}

void EpollTCPServer::Stop() {
  stop_ = true;
  if (mode_ == EpollRunMode::UseProcess) {
    for (auto& info : epoll_process_info_vec_) {
      kill(info.process_id, SIGTERM);
      int state;
      waitpid(info.process_id, &state, 0);
    }
  } else if (mode_ == EpollRunMode::UseThread) {
    for (auto& info : epoll_thread_info_vec_) {
      info.thread_instance.join();
    }
  }
}

ReturnCode EpollTCPServer::InitListenSocket() {
  listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ < 0) {
    LOG_ERROR("Socket Error");
    return ReturnCode::CREATE_SOCK_ERROR;
  }

  struct sockaddr_in server;
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(listen_port_);
  server.sin_addr.s_addr = inet_addr(listen_addr_.c_str());

  bool flag = true;
  int l = sizeof(flag);
  setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &flag, l);
  int ss = bind(listen_fd_, (struct sockaddr*)&server, sizeof(server));
  if (ss < 0) {
    LOG_ERROR("Bind Error");
    return ReturnCode::BIND_ERROR;
  }
  ss = set_nonblock(listen_fd_);
  if (ss < 0) {
    return ReturnCode::FCNTL_ERROR;
  }

  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.data.fd = listen_fd_;
  ev.events = EPOLLIN | EPOLLET;
  ss = epoll_ctl(main_ep_fd_, EPOLL_CTL_ADD, listen_fd_, &ev);
  if (ss < 0) {
    LOG_ERROR("Epoll Add Error");
    return ReturnCode::EP_CONTROL_ERROR;
  }
  LOG_SUCCESS("Create listen socket success");
  return ReturnCode::SUCCESS;
}

void EpollTCPServer::CreateThreads() {
  for (int i = 0; i < parallel_num_; ++i) {
    int cur_pair[2];
    int state = socketpair(AF_UNIX, SOCK_STREAM, 0, cur_pair);
    if (state != 0) {
      LOG_ERROR("socketpair error");
      continue;
    }
    ThreadInfo info;
    info.thread_instance = std::thread(&EpollTCPServer::MainWorker, this, cur_pair[1]);
    info.pair_fd = cur_pair[0];
    epoll_thread_info_vec_.emplace_back(std::move(info));
  }
}

void EpollTCPServer::CreateProcesses() {
  pid_t pid;
  for (int i = 0; i < parallel_num_; ++i) {
    int cur_pair[2];
    int state = socketpair(AF_UNIX, SOCK_STREAM, 0, cur_pair);
    if (state != 0) {
      LOG_ERROR("socketpair error");
      continue;
    }
    if ((pid = fork()) == 0) {
      close(cur_pair[0]);
      MainWorker(cur_pair[1]);
      break;
    } else if (pid > 0) { //for parent process
      close(cur_pair[1]);
      ProcessInfo info{pid, cur_pair[0]};
      epoll_process_info_vec_.emplace_back(std::move(info));
    }
  }
}

void EpollTCPServer::MainWorker(int pair_fd) {
  int thread_ep = epoll_create1(0);
  if (thread_ep < 0) {
    LOG_ERROR("Epoll Create Error");
    return;
  }

  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.data.fd = pair_fd;
  ev.events = EPOLLIN | EPOLLET;

  int ss = epoll_ctl(thread_ep, EPOLL_CTL_ADD, pair_fd, &ev);
  if (ss < 0) {
    LOG_ERROR("Epoll Add Error");
    return;
  }

  struct epoll_event events[kEventLen];
  while (!stop_) {
    int num = epoll_wait(thread_ep, events, kEventLen, -1);
    for (int i = 0; i < num; ++i) {
      if (events[i].data.fd == pair_fd) {
        int client_fd;
        if (mode_ == EpollRunMode::UseProcess) {
          client_fd = recv_fd(pair_fd);
          LOG_DEBUG("Clild Process recv fd: %d", client_fd);
        } else if (mode_ == EpollRunMode::UseThread) {
          char buf[16];
          memset(buf, 0, 16);
          if (read(pair_fd, buf, 16) < 0) {
            continue;
          }
          std::string fd_str{buf};
          client_fd = std::stoi(fd_str);
        }
        struct epoll_event new_ev;
        memset(&ev, 0, sizeof(new_ev));
        new_ev.data.fd = client_fd;
        new_ev.events = EPOLLIN | EPOLLET;
        if ((ss = epoll_ctl(thread_ep, EPOLL_CTL_ADD, client_fd, &new_ev)) < 0) {
          LOG_ERROR("Epoll Add Error");
          continue;
        }
      } else {   // read from clien_fd
        int buf_size = 2048; // enough for client data
        int client_fd = events[i].data.fd;
        struct iovec read_vec;
        char buf_vec[buf_size];
        memset(buf_vec, 0, buf_size);
        read_vec.iov_base = buf_vec;
        read_vec.iov_len = buf_size;
        int n_read = readv(client_fd, &read_vec, 1);
        if (n_read < 0 && errno != EAGAIN) {
          LOG_ERROR("Read From Client Error");
        } else if (n_read == 0) {
          LOG_INFO("Client: [%d] close connection.", client_fd);
        } else {
          int idx = n_read / buf_size + 1;
          for (int i = 0; i < idx; ++i) {
            //LOG_INFO("Index: %d, Data: %s.", i, read_vec[i].iov_base);
            callback_func_(buf_vec, n_read);
          }
        }
      }
    }
  }
}

void EpollTCPServer::StartMainEpoll() {
  int state = listen(listen_fd_, kListenBackLog);
  if (state != 0) {
    LOG_ERROR("Listen Error");
    perror("Listen Error");
    Stop();
    return;
  }

  struct epoll_event events[kEventLen];
  LOG_INFO("Start Epoll Loop");
  while (!stop_) {
    int nums = epoll_wait(main_ep_fd_, events, kEventLen, -1);
    for (int i = 0; i < nums; ++i) {
      if (events[i].data.fd == listen_fd_) {
        struct sockaddr_in cli;
        socklen_t len = sizeof(struct sockaddr_in);
        memset(&cli, 0, sizeof(cli));
        int client_fd = 0;
        while ((client_fd = accept(listen_fd_, (struct sockaddr*)&cli, &len)) > 0) {
          LOG_INFO("Accept client: %d", client_fd);
          set_nonblock(client_fd);
          DealClientConnection(client_fd);
        }
        if (client_fd < 0 && errno != EAGAIN) {
          LOG_ERROR("Accept Error");
        }
      } else { // only enter with UseSingleThread Mode
        int client_fd = events[i].data.fd;
        // read data from client_fd;
      }
    }
  }
}

int EpollTCPServer::GetNextWorkerIndex() {
  if (current_worker_idx_ == parallel_num_) {
    current_worker_idx_ = 0;
  }
  return current_worker_idx_++;
}

void EpollTCPServer::DealClientConnection(int client_fd) {
  if (mode_ == EpollRunMode::UseProcess) {
    SendToChildProcess(client_fd);
    close(client_fd);
  } else if (mode_ == EpollRunMode::UseThread) {
    SendToChildThread(client_fd);
  } else if (mode_ == EpollRunMode::UseSingleThread) {
    AddToMainEpoll(client_fd);
  }
}

void EpollTCPServer::SendToChildProcess(int client_fd) {
  ProcessInfo& info = epoll_process_info_vec_[GetNextWorkerIndex()];
  send_fd(info.pair_fd, client_fd);
}

void EpollTCPServer::SendToChildThread(int client_fd) {
  ThreadInfo& info = epoll_thread_info_vec_[GetNextWorkerIndex()];
  std::string client_fd_str = std::to_string(client_fd);
  int status = write(info.pair_fd, client_fd_str.c_str(), client_fd_str.size());
  if (status <=0) {
    LOG_ERROR("Write Error");
  }
}

void EpollTCPServer::AddToMainEpoll(int client_fd) {
  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.data.fd = client_fd;
  ev.events = EPOLLIN | EPOLLET;
  int ss = epoll_ctl(main_ep_fd_, EPOLL_CTL_ADD, client_fd, &ev);
  if (ss < 0) {
    LOG_ERROR("Epoll Add Error");
  }
}

} //namespace ftcp
