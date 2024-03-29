/*
DO NOT modify this file
this file is generated by
```
python3 gen.py gen-return-code
```

If you want to add return code, modify python code and re-gen.

*/

#ifndef __RETURN_CODE_H_
#define __RETURN_CODE_H_

#include <unordered_map>
#include <string>

namespace ftcp {

enum class ReturnCode {
  SUCCESS = 0,
  EP_CREATE_ERROR = 1,
  EP_CONTROL_ERROR = 1,
  FCNTL_ERROR = 2,
  CREATE_SOCK_ERROR = 3,
  BIND_ERROR = 4,
  SETSOCKOPT_ERROR = 5,
  BIND_DEVICE_ERROR = 6,
  IOCTL_ERROR = 7,
};

static std::unordered_map<ReturnCode, std::string> ret_msg {
  { ReturnCode::SUCCESS, "success" },
  { ReturnCode::EP_CREATE_ERROR, "epoll create error" },
  { ReturnCode::EP_CONTROL_ERROR, "epoll controll error" },
  { ReturnCode::FCNTL_ERROR, "fcntl create error" },
  { ReturnCode::CREATE_SOCK_ERROR, "create socket error" },
  { ReturnCode::BIND_ERROR, "socket bind error" },
  { ReturnCode::SETSOCKOPT_ERROR, "setsockopt error" },
  { ReturnCode::BIND_DEVICE_ERROR, "socket bind device error" },
  { ReturnCode::IOCTL_ERROR, "ioctl error" },
};

} //namespace ftcp

#endif