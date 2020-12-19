'''
发送以太网数据包

python3 send_ether.py -i eth0 -t 00:15:5d:2d:18:f0 -T 0x1024 -d "Hello, world!"
'''
from argparse import ArgumentParser
import base64
import struct
from fcntl import ioctl
from socket import AF_PACKET, SOCK_RAW, AF_INET, SOCK_DGRAM, socket


def parse_arguments():
    '''
    解析命令行参数
    '''
    parser = ArgumentParser(description='Send ethernet frame.')
    # name of iface to send
    parser.add_argument(
        '-i',
        '--iface',
        dest='iface',
        default='eth0',
        required=False,
    )
    # dest mac address
    parser.add_argument(
        '-t',
        '--to',
        dest='to',
        default='00:15:5d:2d:18:f0',
        required=False
    )
    # data to send
    parser.add_argument(
        '-d',
        '--data',
        default='a'*46,
        required=False
    )
    # protocal type
    parser.add_argument(
        '-T',
        '--type',
        dest='_type',
        default='0x0900',  # ip 协议类型
        required=False
    )
    args = parser.parse_args()
    return args


def mac_ntoa(n):
    '''
    将 MAC 地址转化为可读字符串
    '''
    return '%02x:%02x:%02x:%02x:%02x:%02x' % tuple(n)


def mac_aton(a):
    '''
    将可读的 MAC 地址转化为二进制 
    '''
    return base64.b16decode(a.upper().replace(':', ''))


def fetch_iface_mac(iface, s=None):
    if not s:
        s = socket(AF_INET, SOCK_DGRAM)
    # 将网卡名转化为字节
    iface_buf = struct.pack('64s', iface.encode('utf8'))
    # 根据系统调用得到 MAC 地址
    # SIOCGIFHWADDR is 0x8927
    mac = ioctl(s.fileno(), 0x8927, iface_buf)[18:24]
    return mac


def send_ether(iface, to, _type, data, s=None):
    if isinstance(to, str):
        to = mac_aton(to)
    if isinstance(data, str):
        data = data.encode('utf8')
    if s is None:
        # 注意：此处的 socket 需要 SOCK_RAW 使用原生的 socket
        s = socket(AF_PACKET, SOCK_RAW)
    s.bind((iface, 0))
    fr = fetch_iface_mac(iface, s)
    # 以太网包
    header = struct.pack('!6s6sH', to, fr, _type)
    frame = header + data
    s.send(frame)


def main():
    args = parse_arguments()

    send_ether(
        iface=args.iface,
        to=args.to,
        _type=eval(args._type),  # eval 将 16 进制转化为数字
        data=args.data
    )


if __name__ == "__main__":
    main()
