'''
ping ICMP 协议实现

python3.9 ping.py '8.8.8.8'
'''


import os
import socket
import struct
import sys
import threading
import time


def calculate_checksum(icmp):
    '''
    计算校验和
    '''
    highs = icmp[0::2]
    lows = icmp[1::2]

    checksum = ((sum(highs) << 8) + sum(lows))

    while True:
        carry = checksum >> 16
        if carry:
            checksum = (checksum & 0xffff) + carry
        else:
            break

    checksum = ~checksum & 0xffff

    return struct.pack('!H', checksum)


def pack_icmp_echo_request(ident, seq, payload):
    pseudo = struct.pack(
        '!BBHHH',
        8, # icmp 消息類型
        0, # code 默認 0
        0, # 校驗和，當前為 0 
        ident, # 標識，當前進程 ID
        seq, # 序列號，如 1
    ) + payload
    checksum = calculate_checksum(pseudo)
    # 計算校驗和後，将原来的校验和 0 替换成现在的校验和
    return pseudo[:2] + checksum + pseudo[4:]


def unpack_icmp_echo_reply(icmp):
    # 这里的 _ 是忽略的校验和
    _type, code, _, ident, seq, = struct.unpack(
        '!BBHHH',
        icmp[:8]
    )
    if _type != 0:
        return
    if code != 0:
        return

    payload = icmp[8:]

    return ident, seq, payload


def send_routine(sock, addr, ident, magic, stop):
    # first sequence no
    seq = 1

    while not stop:
        # currrent time
        sending_ts = time.time()

        # packet current time to payload
        # in order to calculate round trip time from reply
        payload = struct.pack('!d', sending_ts) + magic

        # pack icmp packet
        icmp = pack_icmp_echo_request(ident, seq, payload)

        # send it
        sock.sendto(icmp, 0, (addr, 0))

        seq += 1
        time.sleep(1)


def recv_routine(sock, ident, magic):
    while True:
        # wait for another icmp packet
        ip, (src_addr, _) = sock.recvfrom(1500)

        # unpack it
        result = unpack_icmp_echo_reply(ip[20:])
        if not result:
            continue

        # print info
        _ident, seq, payload = result
        if _ident != ident:
            continue

        sending_ts, = struct.unpack('!d', payload[:8])
        print('%s seq=%d %5.2fms' % (
            src_addr,
            seq,
            (time.time()-sending_ts) * 1000,
        ))


def ping(addr):
    # create socket for sending and receiving icmp packet
    sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_ICMP)

    # id field
    ident = os.getpid()
    # magic string to pad
    magic = b'1234567890'

    # sender thread stop flag
    # append anything to stop
    sender_stop = []

    # start sender thread
    # call send_routine function to send icmp forever
    args = (sock, addr, ident, magic, sender_stop,)
    sender = threading.Thread(target=send_routine, args=args)
    sender.start()

    try:
        # receive icmp reply forever
        recv_routine(sock, ident, magic)
    except KeyboardInterrupt:
        pass

    # tell sender thread to stop
    sender_stop.append(True)

    # clean sender thread
    sender.join()

    print()


if __name__ == '__main__':
    ping('8.8.8.8')
