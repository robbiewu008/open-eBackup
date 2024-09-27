# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
import sys
import time

import adaptor
from constants import *
from log import logger
from ssh import SSH


class DeployProxy(object):
    def __init__(self, host, username, password, is_windows=False):
        if is_windows:
            # self.con = IWmi(host, username, password)
            self.os_type = Win
            self.os_adaptor = adaptor.Windows()
        else:
            self.con = SSH(host, username, password)
            self.os_type = LINUX
            self.os_adaptor = adaptor.Linux()
            self.set_linux_os_adaptor()
        self.agent_user = AGENT_USER
        self.agent_pwd = AGENT_PWD
        self.host = host
        self.os_adaptor.set_connection(self.con)
        self.install_path = self.os_adaptor.get_install_path()
        self.package_path = self.os_adaptor.get_package_path()
        self.agent_nginx_port = AGENT_PORT

    def close_connection(self):
        self.con.close()
        logger.info('Close connection with host.')

    def stop_agent(self):
        logger.info('begin to stop agent service.')
        self.con.exec('su - rdadmin')
        self.con.exec('cd Agent/bin')
        output = self.con.exec('sh agent_stop.sh')

        if output.lower().find('stopped successfully') != -1:
            logger.info('stop agent successfully.')
            self.con.exec('exit')
            return True
        # stop agent failed.
        logger.info('stop agent failed, message is:%s' % output)
        self.con.exec('exit')
        return False

    def uninstall_agent(self):
        install_file = self.install_path + '/bin/agent_install*'
        if not self.os_adaptor.is_file_exist(install_file):
            self.os_adaptor.clear_install_path()
            logger.info('agent had been uninstalled, clean install path finished.')
            return True
        self.con.exec('cd %s/bin' % self.install_path)
        self.con.exec('cd bin', '#')
        self.con.exec(self.os_adaptor.get_uninstall_cmd(), '>>')
        self.con.exec('y', '#')

        # uninstall success, clear all the file in install path
        self.os_adaptor.clear_install_path()

        logger.info('uninstall agent success.')
        return True

    def copy_file(self):
        history_install = self.os_adaptor.copy_install_file(self.install_path, self.package_path).lower()
        if history_install.find('does not exist') != -1 \
                or history_install.find('not fund') != -1:
            return False

        logger.info('copy file finished, progress is:%s' % history_install)
        return True

    def get_ip_section(self, source_input):
        """
        input like that:
        Please choose IP Address binded by nginx.
        Please select the serial number is greater than 0 Numbers!
            1  10.169.178.207
        >>
        obtain ip address item
        :return:
        """
        lines = source_input.split('\n')
        for line in lines:
            if line.find(self.host) != -1:
                return line.strip().split()[0]
        return '1'

    def install_agent(self):
        history_install = ''
        logger.info('begin to install agent in path [%s]' % self.install_path)
        history_install += self.con.exec('cd %s' % self.install_path)
        history_install += '\n'
        out = self.os_adaptor.un_tar_file().lower()
        if out.find('no such file or directory') != -1 \
                or out.find('cannot found') != -1:
            logger.info('unzip install file failed, message is: %s' % out)
            return False

        history_install += out
        history_install += '\n'

        # change time to 2011
        history_install += self.os_adaptor.pre_treat()
        history_install += '\n'

        history_install += self.con.exec('cd %s/bin' % self.install_path, '#')
        history_install += '\n'
        out = self.con.exec(self.os_adaptor.get_install_cmd(), '>>')
        history_install += out
        if self.os_type == Win and out.lower().find('press any key to continue') != -1:
            # rdadmin exist in windows
            self.con.exec('\n')
            return False

        # rdadmin user is not exist
        if history_install.find('Agent working user rdadmin is not exist') != -1:
            logger.info('Agent working user rdadmin is not exist, install agent failed')
            return False
        history_install += '\n'
        logger.info(history_install)
        history_install += self.con.exec(self.agent_user, ':')
        history_install += '\n'
        history_install += self.con.exec(self.agent_pwd, ':')
        history_install += '\n'
        history_install += self.con.exec(self.agent_pwd, '>>')
        history_install += '\n'

        # ipv4 or ipv6
        ip_address_list = self.con.exec('1', '>>')
        history_install = ip_address_list + '\n'

        if history_install.lower().find('failed to obtain the ip address') != -1:
            return False

        ip_address_section = self.get_ip_section(ip_address_list)
        # select net interface
        history_install += self.con.exec(ip_address_section, '>>')
        history_install += '\n'

        # Please input rdagent listening port number 1024-65535, default port 8091:
        listen_port_out = self.con.exec('\n', '>>')

        history_install += listen_port_out

        # Please input nginx listening port number 1024-65535, default port 59526:
        output = self.con.exec('\n', '#')
        if output.lower().find('is used by other process') != -1:
            return False
        # OceanStor BCManager Agent was installed successfully.
        if output.lower().find('installed successfully.') != -1 or history_install.find(
                'installed successfully.') != -1:
            return True

        # install failed
        history_install += output
        logger.info('install agent failed, message is: %s' % history_install)
        return False

    def check_agent_status(self):
        logger.info('begin check agent status...')
        prompt = self.os_adaptor.get_prompt()

        output = self.con.exec('ps -ef|grep rdagent|grep -v grep', prompt)
        # for Sun Microsystems, it display /export/home/rdadmin//Agent/bin/rdagent
        output = output.replace("//", '/')
        if output.find('%s/bin rdagent' % self.install_path) != -1 or output.find(
                '%s/bin/rdagent' % self.install_path) != -1:
            logger.info('rdagent was start successfully.')
            return True
        # start agent failed
        logger.info('rdagent was start failed, message is: %s' % output)
        return False

    def start_agent(self):
        logger.info('begin to start agent')
        su_prompt = self.os_adaptor.get_su_prompt()
        self.os_adaptor.enter_bash()
        history = ''
        history += self.con.exec('su - rdadmin', su_prompt)
        history += self.con.exec('cd Agent/bin', su_prompt)
        output = self.con.exec(self.os_adaptor.get_start_cmd(), su_prompt, timeout=15)
        start_success = False
        if output.lower().find('started successfully.') != -1:
            start_success = True
        history += output
        # exit to root
        output = self.con.exec('exit', self.os_adaptor.get_prompt())
        history += output

        history += self.os_adaptor.post_treat()
        history += '\n'

        return start_success

    def get_os_type(self):
        out_str = str(self.con.exec('uname -a', '#')).lower()
        if out_str.find('linux') != -1:
            self.os_type = LINUX
        if out_str.find('hp-ux') != -1:
            self.os_type = HP
        if out_str.find('aix') != -1:
            self.os_type = AIX

    def create_install_path(self):
        if self.os_adaptor.is_file_exist(self.install_path):
            return
        out = self.os_adaptor.create_install_path()
        logger.info('Create install path finished. %s' % out)

    def set_linux_os_adaptor(self):
        out_str = str(self.con.exec('uname -a', '#')).lower()
        if out_str.find('linux') != -1:
            self.os_type = LINUX
            self.os_adaptor = adaptor.Linux()
        if out_str.find('hp-ux') != -1:
            self.os_type = HP
            self.os_adaptor = adaptor.HP()
        if out_str.find('aix') != -1:
            self.os_type = AIX
            self.os_adaptor = adaptor.AIX()
        if out_str.find('sunos') != -1:
            self.os_type = SunOS
            self.os_adaptor = adaptor.SunOS()
        if out_str.find('rocky6') != -1:
            self.os_type = Rocky6
            self.os_adaptor = adaptor.Rocky6()


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('usage: python %s host_ip user pwd install_path' % sys.argv[0])
        exit(-1)
    host_ip = sys.argv[1]
    host_user_name = sys.argv[2]
    host_user_pwd = sys.argv[3]
    if len(sys.argv) > 4:
        is_win = sys.argv[4]
        proxy = DeployProxy(host_ip, host_user_name, host_user_pwd, is_windows=is_win)
    else:
        proxy = DeployProxy(host_ip, host_user_name, host_user_pwd)

    if not proxy.uninstall_agent():
        exit(-1)
    proxy.create_install_path()
    # waiting the port from status FIN_WAIT2 to terminal.
    time.sleep(2)
    if not proxy.copy_file():
        exit(-2)
    if not proxy.install_agent():
        exit(-3)
    if not proxy.start_agent():
        exit(-4)
    proxy.check_agent_status()
    proxy.close_connection()
