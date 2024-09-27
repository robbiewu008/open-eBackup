# coding=utf-8
import datetime
import time

import os

from log import logger


class OSAdaptor(object):
    def __init__(self):
        self.con = None

    def set_connection(self, connection):
        self.con = connection

    def get_su_prompt(self):
        return '$'

    def get_prompt(self):
        return '#'

    def is_file_exist(self, file_path):
        output = self.con.exec('ls ' + file_path, except_str=self.get_prompt()).lower()
        if output.find('not found') != -1 or \
                output.find('does not exist') != -1 or \
                output.find('no such file or directory') != -1 or \
                output.find(r'无法访问') != -1:
            return False
        # File exists
        return True

    def create_install_path(self):
        output = ''
        output += self.con.exec('mkdir %s' % self.get_install_path(), self.get_prompt())
        output += self.con.exec('mkdir %s/Agent' % self.get_install_path(), self.get_prompt())
        return output

    def un_tar_file(self):
        self.con.exec('cd %s' % self.get_install_path(), except_str=self.get_prompt())
        return self.con.exec('tar -xvzf *.tar.gz', except_str=self.get_prompt())

    def get_install_path(self):
        return '/home/rdadmin/Agent'

    def get_package_path(self):
        return '/home/package'

    def copy_install_file(self, install_path, source_package_path):
        history_install = ''
        history_install += self.con.exec('cd %s' % install_path, self.get_prompt())
        history_install += self.con.exec('cp %s/*.tar.gz ./' % source_package_path, self.get_prompt())
        if history_install.lower().find('overwrite') != -1:
            history_install += self.con.exec('\n')
        #
        return history_install

    def get_install_cmd(self):
        return 'sh agent_install.sh'

    def get_start_cmd(self):
        return 'sh agent_start.sh'

    def get_uninstall_cmd(self):
        return 'sh agent_uninstall.sh'

    def clear_install_path(self):
        return self.con.exec('rm -rf %s/*' % self.get_install_path())

    def enter_bash(self):
        return ''

    def pre_treat(self):
        retry_time = 0
        while True:
            if retry_time >= 3:
                break
            ret = self.con.exec('netstat -an|grep \"127.0.0.1:8091\"')
            if ret.find('TIME_WAIT') < 0:
                break
            logger.info('the 8091 port used by other process, wait for 30 seconds, retry times [%s]' % retry_time)
            time.sleep(30)
            retry_time += 1
        return ''

    def post_treat(self):
        return ''

    def exec_cmd_with_ret(self, cmd):
        return self.con.exec(cmd);


class HP(OSAdaptor):
    def get_su_prompt(self):
        return '$'


class Oracle(OSAdaptor):
    def get_su_prompt(self):
        return '#'


class Linux(OSAdaptor):
    def get_su_prompt(self):
        return '$'


class AIX(OSAdaptor):
    def un_tar_file(self):
        un_tar_history = ''
        un_tar_history += self.con.exec('cd %s' % self.get_install_path(), except_str=self.get_prompt())
        un_tar_history += self.con.exec('gunzip *.tar.gz -f', except_str=self.get_prompt())
        un_tar_history += self.con.exec('tar -xvf *.tar', except_str=self.get_prompt())
        return un_tar_history


class SunOS(OSAdaptor):
    def get_install_path(self):
        return '/export/home/rdadmin/Agent'

    def get_package_path(self):
        return '/export/home/workspace/Package_RD_Agent_Solaris_V2R1C30/AGENT_PACK_TEMP'

    def copy_install_file(self, install_path, source_package_path):
        install_cmd_history = ""
        install_cmd_history += super().copy_install_file(install_path, source_package_path)
        # delete resource package
        install_cmd_history += self.con.exec('rm -rf *SRC*', self.get_prompt())
        return install_cmd_history

    def un_tar_file(self):
        cmd_history = ''
        cmd_history += self.exec_cmd_with_ret('cd %s' % self.get_install_path(), except_str=self.get_prompt())
        cmd_history += self.exec_cmd_with_ret('gunzip *.tar.gz -f', except_str=self.get_prompt())
        cmd_history += self.exec_cmd_with_ret('tar -xvf *.tar', except_str=self.get_prompt())
        return cmd_history

    def enter_bash(self):
        self.con.exec('/bin/bash')


class Rocky6(OSAdaptor):
    def pre_treat(self):
        return self.con.exec('date -s \"01 Jan 2011 00:00:00\"', except_str=self.get_prompt())

    def post_treat(self):
        now = datetime.datetime.now().strftime('%Y-%m-%d %H:%M')
        return self.con.exec('date -s \"%s\"' % now, except_str=self.get_prompt())

    def get_su_prompt(self):
        return '%'


class Windows(OSAdaptor):
    def __init__(self):
        self.cmd_path = 'cmd.exe /c %s%s%s%s' % (self.get_install_path(), os.sep, 'bin', os.sep)

    def is_file_exist(self, file_path):
        output = self.con.exec('dir %s' % file_path).lower()
        if output.find('not found') != -1 or \
                output.find('does not exist') != -1 or \
                output.find('no such file or directory') != -1 or \
                output.find('cannot find') != -1:
            return False
        # File exists
        return True

    def get_install_path(self):
        return r'D:\agent_install_for_auto_test\Agent'

    def get_package_path(self):
        return r'd:\package'

    def create_install_path(self):
        output = self.con.exec('mkdir %s' % self.get_install_path(), self.get_prompt())
        return output

    def copy_install_file(self, install_path, source_package_path):
        history_install = ''
        history_install += self.con.exec('cd %s' % install_path, self.get_prompt())
        history_install += self.con.exec('xcopy %s%s*.zip %s /y' % (source_package_path, os.sep, install_path),
                                         self.get_prompt())
        return history_install

    def un_tar_file(self):
        return self.con.exec('unzip -o ' + self.get_install_path() + os.sep + '*.zip -d %s' % self.get_install_path())

    def get_install_cmd(self):
        return self.cmd_path + 'agent_install.bat'

    def get_start_cmd(self):
        return self.cmd_path + 'agent_start.bat'

    def get_uninstall_cmd(self):
        return self.cmd_path + 'agent_uninstall.bat'

    def clear_install_path(self):
        return self.con.exec('rd /q /s ' + self.get_install_path())


if __name__ == '__main__':
    hp = HP()
    oracle = Oracle()
