import socket
import time

import paramiko

from connection import IConnection
from log import logger


class SSH(IConnection):
    def __init__(self, host, username, password, port=22):
        self.ssh = paramiko.SSHClient()
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.ssh.connect(host, port, username, password)
        self.host = host
        self.linesep = '\n'
        self.channel = self.ssh.invoke_shell(width=200, height=200)
        result, is_match = self.__receive(timeout=5)
        logger.info('login info: %s' % result)

    def send_msg(self, msg, except_str='#'):
        return self.exec(msg, except_str)

    def exec(self, cmd, except_str='#', timeout=10):
        now_time = time.time()
        end_time = now_time + timeout
        channel = self.channel
        while now_time < end_time:
            try:
                channel.send(cmd + self.linesep)
                time.sleep(1)
                receive, is_match = self.__receive(except_str)
                logger.info('remote ip[%s], exec cmd[%s], receive[%s]' % (self.host, cmd, receive))
                return receive
            except socket.timeout as e:
                logger.info(e, "execute cmd [%s] to host [%s] timeout." % (cmd, self.host))
            now_time = time.time()
        return None

    def __receive(self, except_str='#', timeout=10):
        is_match = False
        receive = ""
        now_time = time.time()
        end_time = now_time + timeout
        channel = self.channel
        while now_time < end_time:
            str_get = ''
            if not self.channel.recv_ready():
                now_time = time.time()
                continue
            try:
                str_get = channel.recv(65535)
                str_get = str_get.decode('utf-8', 'ignore')
            except socket.timeout:
                logger.info('socket time out while waiting receive from host[%s]' % self.host)
            except UnicodeDecodeError as msg:
                logger.info('can not decode byte error from host[%s]' % self.host, msg)
            if str_get is not '':
                receive += str_get
            if receive.rfind(except_str) != -1:
                is_match = True
                break
            now_time = time.time()
        return receive, is_match

    def close(self):
        self.ssh.close()


if __name__ == '__main__':
    # hp
    ssh = SSH('10.158.192.230', 'root', 'Cloud12#$')
    # ssh = SSH('10.183.171.238', 'root', 'apple')
    out = ssh.exec('ls /home/rdadmin/Agent/aaa')
    print(out)
