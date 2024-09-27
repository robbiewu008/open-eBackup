import inspect
import json
import sys

from constants import *
from rest import *

host_ip = ''
host_user_name = ''
host_user_pwd = ''


class Cerberus(object):
    def __init__(self, ip_address, u_name, password, port=AGENT_PORT):
        self.ip = ip_address
        self.user = u_name
        self.pwd = password
        self.port = port
        self.http = Http()

    def query_host(self):
        url = 'https://%s:%s/agent/host' % (self.ip, self.port)
        status, resp_content = self.http.get(url, self.user, self.pwd)
        if status and resp_content:
            host_agent_info = json.loads(resp_content)
            if status == 200:
                return host_agent_info
            logger.info('query agent status error, status: %d, response: %s' % (status, resp_content))
            return None
        else:
            return None

    def test_status(self):
        host_info = self.query_host()
        if host_info:
            logger.info('Agent is active in host[%s], host info[%s]' % (host_ip, host_info))
            return True
        # agent status is inactive
        logger.info('Agent is inactive in host[%s], host info[%s]' % (host_ip, host_info))
        return False

    def test_fun1(self):
        """
        any test case must start with test_
        :return:any test case must return True of False
        """

        logger.info('test_fun1 invoked, host agent ip[%s]!' % self.ip)
        return True

    def test_fun2(self):
        logger.info('test_fun1 invoked, host agent ip[%s]!' % self.ip)
        return True


if __name__ == '__main__':

    if len(sys.argv) < 3:
        print('usage: python %s host_ip user pwd' % sys.argv[0])
        exit(-1)
    host_ip = sys.argv[1]
    host_user_name = sys.argv[2]
    host_user_pwd = sys.argv[3]
    cerberus = Cerberus(host_ip, host_user_name, host_user_pwd)
    is_all_test_case_passed = 0
    for name, func in inspect.getmembers(cerberus):
        try:
            if name.startswith('test_'):
                if not func():
                    # if any test case failed, the whole pip line will be failed.
                    is_all_test_case_passed = -1
        except Exception as error:
            logger.debug('execute test method[%s] error' % name)
            is_all_test_case_passed = -1
    # exit no zero code, the cloud dragon will fetch the result
    exit(is_all_test_case_passed)
