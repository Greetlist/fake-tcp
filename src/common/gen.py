from jinja2 import Environment, FileSystemLoader
import argh
import os

def gen_return_code():
    # (enum_name, enum_value, enum_error_string)
    return_code_list = [
        {'name': "SUCCESS", 'value': 0, 'msg': "success"},
        {'name': "EP_CREATE_ERROR", 'value': 1, 'msg': "epoll create error"},
        {'name': "EP_CONTROL_ERROR", 'value': 1, 'msg': "epoll controll error"},
        {'name': "FCNTL_ERROR", 'value': 2, 'msg': "fcntl create error"},
        {'name': "CREATE_SOCK_ERROR", 'value': 3, 'msg': "create socket error"},
        {'name': "BIND_ERROR", 'value': 4, 'msg': "socket bind error"},
        {'name': "SETSOCKOPT_ERROR", 'value': 5, 'msg': "setsockopt error"},
        {'name': "BIND_DEVICE_ERROR", 'value': 6, 'msg': "socket bind device error"},
        {'name': "IOCTL_ERROR", 'value': 7, 'msg': "ioctl error"},
    ]
    template_file = './return_code.h.j2'

    env = Environment(
        loader=FileSystemLoader(os.path.dirname(template_file)),
        trim_blocks=True
    )
    res = env.get_template(os.path.basename(template_file)).render({'return_code_list': return_code_list}).encode('utf-8')
    with open('./return_code.h', 'w') as f:
        f.write(res.decode('utf-8'))

if __name__ == '__main__':
    argh.dispatch_commands([gen_return_code])
