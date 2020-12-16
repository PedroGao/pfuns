# 查询本机的 MAC 地址
# 用法：第一个参数是网卡名称
# 推荐使用 `ip addr` 来获取网卡名词
# python3 show_mac.py enp2s0
# iFace: enp2s0
# MAC: b0:83:fe:70:fb:b5

import base64
import struct
import sys

from fcntl import ioctl
from socket import AF_INET, SOCK_DGRAM, socket


def mac_ntoa(n):
    '''
    将 MAC 二进制转化为可读的 16 进制字符串
    '''
    return '%02x:%02x:%02x:%02x:%02x:%02x' % tuple(n)


def fetch_iface_mac(iface, s=None):
    if not s:
        s = socket(AF_INET, SOCK_DGRAM)
    # 将网卡名转化为字节
    iface_buf = struct.pack('64s', iface.encode('utf8'))
    # 根据系统调用得到 MAC 地址
    # SIOCGIFHWADDR is 0x8927
    mac = ioctl(s.fileno(), 0x8927, iface_buf)[18:24]
    return mac


def main():
    iface = sys.argv[1]
    mac = fetch_iface_mac(iface)
    print('iFace: {iface}'.format(iface=iface))
    print('MAC: {mac}'.format(mac=mac_ntoa(mac)))


if __name__ == "__main__":
    main()
