# coding: utf-8
import json
import os
import re
import shlex
import subprocess
import sys
import time
import shutil
import grp
import pexpect

from app.common.common import release_str_memory, get_data_from_api, initialize_kmc, check_ip_address
from app.common.const import K8sConst, SftpConst, CustomErrorCode, InfraUrlConstant, HttpConst
from app.common.logger import log

HOST_KEY_PATH = '/etc/ssh'


def get_sftp_ip():
    sftp_ip = ""
    try:
        initialize_kmc()
        url = InfraUrlConstant.GET_CONFIG + f"?nameSpace={K8sConst.NAMESPACE}&configMap={K8sConst.COMMON_CONF}"
        response = get_data_from_api(url, "GET")
        if response.status != HttpConst.OK:
            # 访问接口失败
            log.error("Request failed.")
            return sftp_ip
        data = json.loads(response.read().decode('utf-8'))
        if data.get("error") is not None:
            log.error("Failed to get the SFTP ip.")
            return sftp_ip
        for items in data.get("data"):
            for key, value in items.items():
                if key == SftpConst.SFTP_IP:
                    sftp_ip = value
                    log.info("Succeed to get the SFTP ip.")
                    break
        return sftp_ip
    except Exception:
        log.exception("Exception when get sftp ip.")
        return sftp_ip


class ServiceUtil(object):
    """工具类"""

    @staticmethod
    def execute_cmd(cmd):
        """执行cmd命令"""
        process_list = []
        # shell=False参数不支持管道
        cmds = cmd.split('|')
        tmp_stdin = None
        for tmp_cmd in cmds:
            process = subprocess.Popen(shlex.split(tmp_cmd), stdin=tmp_stdin, stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE, encoding="utf-8")
            process.wait()
            tmp_stdin = process.stdout
            process_list.append(process)

        return str(process_list[-1].returncode), process_list[-1].stdout.read(), process_list[-1].stderr.read()

    @staticmethod
    def execute_cmd_list(cmds):
        """执行cmd命令，不以管道符进行切割命令，传入命令未list"""
        process_list = []
        # shell=False参数不支持管道
        tmp_stdin = None
        for tmp_cmd in cmds:
            process = subprocess.Popen(shlex.split(tmp_cmd), stdin=tmp_stdin, stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE, encoding="utf-8")
            process.wait()
            tmp_stdin = process.stdout
            process_list.append(process)

        return str(process_list[-1].returncode), process_list[-1].stdout.read(), process_list[-1].stderr.read()

    @staticmethod
    def list_group_names():
        """列出所有组名"""
        return_code, std_out, std_err = ServiceUtil.execute_cmd('''cat /etc/group | cut -f 1 -d :''')
        if return_code != "0":
            return return_code, std_out, std_err
        if not std_out:
            # 没有成员
            return return_code, list(), ""
        group_list = std_out.split("\n")[:-1]
        return return_code, group_list, ""

    @staticmethod
    def list_user_names(group_name):
        """列出所有用户名"""
        cmd = '''awk -F: 'BEGIN{gid=ARGV[2]; delete ARGV[2];} {if ($4 == gid) print $1;}' /etc/passwd '''
        cmd = "{}{}".format(cmd, grp.getgrnam(group_name).gr_gid)
        return_code, std_out, std_err = ServiceUtil.execute_cmd(cmd)
        if return_code != "0":
            return return_code, std_out, std_err
        if not std_out:
            # 没有成员
            return return_code, list(), ""
        user_list = std_out.split("\n")[:-1]
        return return_code, user_list, ""

    @staticmethod
    def add_group_name(group_name):
        """增加组"""
        return_code, std_out, std_err = ServiceUtil.list_group_names()
        if return_code != "0":
            log.error("List group name group_name:[%s] command failed." % group_name)
            return return_code, std_out, std_err
        else:
            group_list = std_out
            if group_name not in group_list:
                # 组sftp不存在，增加组
                return_code, std_out, std_err = ServiceUtil.execute_cmd('''groupadd %s''' % group_name)
                if return_code != "0":
                    log.error("Add group name group_name:[%s] command failed." % group_name)
                    return return_code, std_out, std_err
        return "0", "", "Add group name [%s] successfully." % group_name

    @staticmethod
    def add_user_name(user_name, group_name, u_id):
        """增加用户"""
        return_code, std_out, std_err = ServiceUtil.list_user_names(group_name)
        if return_code != "0":
            log.error("List user name user_name:[%s] command failed." % user_name)
            return return_code, std_out, std_err
        else:
            user_list = std_out
            if user_name not in user_list:
                # 用户不存在，增加用户
                if not u_id:
                    cmd = '''useradd -g %s -s /bin/false %s''' % (group_name, user_name)
                else:
                    cmd = '''useradd -g %s -u %s -s /bin/false %s''' % (group_name, u_id, user_name)
                return_code, std_out, std_err = ServiceUtil.execute_cmd_list([cmd])
                if return_code != "0":
                    log.error("Add user name user_name:[%s] command failed." % user_name)
                    return return_code, std_out, std_err
        return "0", "", "Add user name [%s] successfully." % user_name

    @staticmethod
    def add_group_and_user(group_name, user_name, u_id, home_path):
        """创建组，增加用户"""
        # 如果sftp组不存在，则创建sftp组
        return_code, std_out, std_err = ServiceUtil.add_group_name(group_name)
        if return_code != "0":
            log.error("Add group name group_name:[%s] failed." % group_name)
            return return_code, std_out, std_err
        log.info("Add group name group_name:[%s] successfully." % group_name)
        # 组sftp已存在，增加用户
        # 如果用户不存在，则增加
        return_code, std_out, std_err = ServiceUtil.add_user_name(user_name, group_name, u_id)
        if return_code != "0":
            log.error("Add user name user_name:[%s] failed." % user_name)
            return return_code, std_out, std_err
        log.info("Add user name user_name:[%s] successfully." % user_name)
        # 设置用户目录为用户的家目录
        return_code, std_out, std_err = ServiceUtil.execute_cmd('''usermod -d %s %s''' % (home_path, user_name))
        if return_code != "0":
            log.error("Set user user_name:[%s] home directory command failed." % user_name)
            return return_code, std_out, std_err
        log.info("Set user user_name:[%s] home directory successfully." % user_name)
        return "0", "", "Add group name %s and user name [%s] successfully." % (group_name, user_name)

    @staticmethod
    def make_and_set(home_path, secondary_path, user_name):
        """创建用户目录，设置用户目录为用户的家目录，设置家目录权限"""
        # 创建用户目录
        if not os.path.exists(secondary_path):
            return_code, std_out, std_err = ServiceUtil.execute_cmd('''mkdir -pv %s''' % secondary_path)
            if return_code != "0":
                log.error("Make user user_name:[%s] directory command failed." % user_name)
                return return_code, std_out, std_err
        log.info(
            "Make user user_name:[%s] directory successfully. user directory is secondary_path:[%s]"
            % (user_name, secondary_path)
        )
        # 创建etc目录
        etc_path = "{}{}".format(home_path, "/etc")
        log.info("User user_name:[%s] etc directory is etc_path:[%s]" % (user_name, etc_path))
        if not os.path.exists(etc_path):
            return_code, std_out, std_err = ServiceUtil.execute_cmd('''mkdir -pv %s''' % etc_path)
            if return_code != "0":
                log.error("Make user user_name:[%s] etc directory command failed." % user_name)
                return return_code, std_out, std_err
        log.info(
            "Make user user_name:[%s] etc directory successfully. user etc directory is etc_path:[%s]"
            % (user_name, etc_path)
        )
        # 同步时区文件
        return_code, std_out, std_err = ServiceUtil.execute_cmd('''cp /etc/localtime %s/localtime''' % etc_path)
        if return_code != "0":
            log.error("Copy system localtime to user_name:[%s] etc directory command failed." % user_name)
            return return_code, std_out, std_err
        log.info("Copy system localtime to user_name:[%s] etc directory successfully." % user_name)
        # 设置家目录权限
        return_code, std_out, std_err = ServiceUtil.execute_cmd('''chmod -R 755 %s''' % home_path)
        if return_code != "0":
            log.error("Set mod of user user_name:[%s] home directory command failed." % user_name)
            return return_code, std_out, std_err
        log.info("Set mod of user user_name:[%s] home directory successfully." % user_name)
        return "0", "", "Make and set owner and mod of user [%s] home directory successfully." % user_name

    @staticmethod
    def modify_ssh_config():
        """修改/etc/ssh/sshd_config配置文件"""
        try:
            with open(SftpConst.SSHD_CONFIG, "r+") as f:
                config_content = f.read()
                new_config = config_content
                # 先全文搜索config_content，不存在目标内容则修改
                # 如果全文都不存在"# Subsystem sftp /usr/libexec/openssh/sftp-server"，则修改
                if not re.search(r"#\s+Subsystem\s+sftp\s+/usr/libexec/openssh/sftp-server", config_content):
                    # 注释掉Subsystem sftp /usr/libexec/openssh/sftp-server
                    target_str = re.search(r"Subsystem\s+sftp\s+/usr/libexec/openssh/sftp-server.*\n",
                                           config_content).group()
                    repl = "{}{}".format("# ", target_str)
                    while True:
                        sftp_ip = get_sftp_ip()
                        if sftp_ip:
                            if check_ip_address(sftp_ip):
                                log.info("The sftp ip(%s) is valid.", sftp_ip)
                                break
                            log.error("The sftp ip(%s) is invalid.", sftp_ip)
                            return False
                        time.sleep(1)
                    repl = "{}{}".format(repl, "Subsystem sftp internal-sftp -l INFO -f AUTH\nListenAddress %s\n"
                                         % sftp_ip)
                    new_config = re.sub(r"Subsystem\s+sftp\s+/usr/libexec/openssh/sftp-server.*\n", repl, new_config)
                if re.search(r"(#RekeyLimit default none\n)", config_content):
                    if not re.search(r"(#RekeyLimit default none\n\nMACs hmac-sha2-256,hmac-sha2-256-etm@openssh.com,"
                                     r"hmac-sha2-512,hmac-sha2-512-etm@openssh.com\n"
                                     r"Ciphers aes128-ctr,aes192-ctr,aes256-ctr,chacha20-poly1305@openssh.com,"
                                     r"aes128-gcm@openssh.com,aes256-gcm@openssh.com\n)",
                                     config_content):
                        repl = "#RekeyLimit default none\n\nMACs hmac-sha2-256,hmac-sha2-256-etm@openssh.com," \
                               "hmac-sha2-512,hmac-sha2-512-etm@openssh.com\nCiphers aes128-ctr,aes192-ctr," \
                               "aes256-ctr,chacha20-poly1305@openssh.com," \
                               "aes128-gcm@openssh.com,aes256-gcm@openssh.com\n"
                        new_config = re.sub(r"(#RekeyLimit default none\n)", repl, new_config)
                # 如果全文都不存在AllowGroups，则新增
                re_pattern = "AllowGroups %s" % SftpConst.SFTP_GROUP_NAME
                if not re.search(re_pattern, config_content):
                    line = "AllowGroups {}\n".format(SftpConst.SFTP_GROUP_NAME)
                    new_config = "{}{}".format(new_config, line)
                # 如果全文都不存在Match Group sftp，则新增
                re_pattern = "Match Group %s" % SftpConst.SFTP_GROUP_NAME
                if not re.search(re_pattern, config_content):
                    line = "{}{}{}{}{}".format("Match Group %s\n" % SftpConst.SFTP_GROUP_NAME,
                                               "    ChrootDirectory /sftp/%u\n",
                                               "    ForceCommand internal-sftp -l INFO -f AUTH\n",
                                               "    AllowTcpForwarding no\n",
                                               "    X11Forwarding no\n")
                    new_config = "{}{}".format(new_config, line)
                # 如果全文都不存在PermitRootLogin no或PermitRootLogin yes，则新增
                if not re.search(r"(PermitRootLogin no\n)|(PermitRootLogin yes\n)", config_content):
                    line = "{}{}".format("PermitRootLogin no", "\n")
                    new_config = "{}{}".format(new_config, line)
                else:
                    # 如果存在，置为PermitRootLogin no
                    repl = "PermitRootLogin no\n"
                    new_config = re.sub(r"(PermitRootLogin no\n)|(PermitRootLogin yes\n)", repl, new_config)
                # 如果全文都不存在StrictModes no或StrictMode yes，则新增StrictModes yes
                if not re.search(r"(StrictModes no\n)|(StrictModes yes\n)", config_content):
                    line = "{}{}".format("StrictModes yes", "\n")
                    new_config = "{}{}".format(new_config, line)
                else:
                    # 如果存在，置为StrictModes yes
                    repl = "StrictModes yes\n"
                    new_config = re.sub(r"(StrictModes no\n)|(StrictModes yes\n)|"
                                        r"(#StrictModes yes\n)|(#StrictModes no\n)", repl, new_config)
                # 将原文件内容清空
                f.seek(0)
                f.truncate()
                # 写入新内容
                f.write(new_config)
            # 修改hostKey
            cmd = '''cat /etc/ssh/sshd_config | grep "HostKey /etc/ssh/"'''
            return_code, std_out, std_err = ServiceUtil.execute_cmd(cmd)
            if std_out:
                cmd = '''sed -i "s#HostKey /etc/ssh#HostKey %s#g" /etc/ssh/sshd_config''' % HOST_KEY_PATH
                return_code, std_out, std_err = ServiceUtil.execute_cmd(cmd)
                if return_code != "0":
                    log.error("Set the path of HostKey failed")
                    return return_code, std_out, std_err
            # 修改端口信息
            cmd = '''sed -i 's/^#Port [0-9]\+$/Port 30177/' /etc/ssh/sshd_config'''
            return_code, std_out, std_err = ServiceUtil.execute_cmd(cmd)
            if return_code != "0":
                log.error("Set the path of HostKey failed")
                return False
            return True
        except Exception as e:
            log.error("Modify sshd config file failed")
            return False

    @staticmethod
    def execute_gen_ssh_key(key_file, key_type):
        """生成ssh_key文件"""
        child_proc = pexpect.spawnu('''ssh-keygen -t %s -f %s''' % (key_type, key_file))
        if child_proc.expect(r"Enter passphrase") == 0:
            child_proc.sendline()
            if child_proc.expect(r"Enter same passphrase again") == 0:
                child_proc.sendline()
                if child_proc.expect(r"Your identification") == 0:
                    child_proc.sendeof()
                else:
                    log.error("Second input for passphrase of ssh key key_file:[%s] failed." % key_file)
                    return (CustomErrorCode.generate_rsa_key_failed, "",
                            "Second input for passphrase of ssh key [%s] failed." % key_file)
            else:
                log.error("First input for passphrase of ssh key key_file:[%s] failed." % key_file)
                return (CustomErrorCode.generate_rsa_key_failed, "",
                        "First input for passphrase of ssh key [%s] failed." % key_file)
        else:
            log.error("Generate ssh key key_file:[%s] command failed." % key_file)
            return (CustomErrorCode.generate_rsa_key_failed, "",
                    "Generate rsa key [%s] command failed." % key_file)
        return "0", "", "Execute generate ssh key [%s] command successfully." % key_file

    @staticmethod
    def generate_ssh_key(ssh_key, ssh_key_type):
        """密钥文件不存在时，执行ssh-keygen命令"""
        if not os.path.exists(HOST_KEY_PATH):
            os.mkdir(HOST_KEY_PATH)
        key_index = (i for i in range(len(ssh_key)))
        for j in key_index:
            if not os.path.exists(ssh_key[j]):
                return_code, std_out, std_err = ServiceUtil.execute_gen_ssh_key(ssh_key[j], ssh_key_type[j])
                if return_code != "0":
                    log.error("Generate ssh key ssh_key:[%s] failed." % ssh_key[j])
                    return return_code, std_out, std_err
                log.info("Generate ssh key ssh_key:[%s] successfully." % ssh_key[j])
            else:
                log.info("Ssh key ssh_key:[%s] exists. No need to generate." % ssh_key[j])
        return "0", "", "Generate ssh key successfully."

    @staticmethod
    def modify_log_config():
        """修改/etc/rsyslog.d/sftpchroot.conf配置文件"""
        try:
            with open(SftpConst.SYSLOGD_CONFIG, "r+") as f:
                # 将原文件内容清空
                f.seek(0)
                f.truncate()
                # 写入新内容
                f.write('''/var/log/messages\n''')
                f.write(''':syslogtag, contains, "internal-sftp" -/var/log/internal-sftp.log\n''')
            return True
        except Exception as e:
            log.error("Modify syslogd config file error failed")
            return False

    @staticmethod
    def set_log_config():
        """配置syslogd"""
        # 不存在syslogd_config文件，先创建
        if not os.path.exists(SftpConst.SYSLOGD_CONFIG):
            return_code, std_out, std_err = ServiceUtil.execute_cmd('''touch /etc/rsyslog.d/sftpchroot.conf''')
            if return_code != "0":
                log.error("Touch syslogd config file command failed.")
                return return_code, std_out, std_err
        else:
            # 已存在syslogd_config，跳过此步
            log.info("Syslogd config file exists. No meed to execute touch.")
        # 修改syslogd_config
        res = ServiceUtil.modify_log_config()
        if not res:
            log.error("Modify syslogd config file failed.")
            return CustomErrorCode.syslogd_config_modify_failed, "", "Modify syslogd config file failed."
        return "0", "", "Generate ssh key successfully."

    @staticmethod
    def kill_and_restart_log():
        """syslogd进程重启, 并查看syslogd是否已成功重启"""
        # 查看syslogd是否已启动
        return_code, std_out, std_err = ServiceUtil.execute_cmd('''ps -ef | grep rsyslogd | grep -v grep''')
        if return_code != "0":
            log.info("Rsyslogd is not at start status.")
        else:
            log.info("Rsyslogd is at start status.")
            # 杀死syslogd进程
            cmd = '''ps -ef |grep rsyslogd |grep -v grep |awk '{print $2}' |xargs kill -9'''
            return_code, std_out, std_err = ServiceUtil.execute_cmd(cmd)
            if return_code != "0":
                log.error("Kill rsyslogd command failed.")
                return return_code, std_out, std_err
        # 重启syslogd进程
        return_code, std_out, std_err = ServiceUtil.execute_cmd('''/usr/sbin/rsyslogd''')
        if return_code != "0":
            log.error("Restart rsyslogd command failed.")
            return return_code, std_out, std_err
        # 再次查看syslogd是否已启动
        return_code, std_out, std_err = ServiceUtil.execute_cmd('''ps -ef | grep rsyslogd | grep -v grep''')
        if return_code != "0":
            log.error("Kill rsyslogd process and restart failed.")
            return return_code, std_out, std_err
        return "0", "", "Kill rsyslogd process and restart successfully."

    @staticmethod
    def kill_and_restart_ssh():
        """sshd进程重启, 并查看sshd是否已成功重启"""
        # 查看sshd是否已启动
        return_code, std_out, std_err = ServiceUtil.execute_cmd('''ps -ef |grep sshd | grep -v grep''')
        if return_code != "0":
            log.info("Sshd is not at start status.")
        else:
            log.info("Sshd is at start status.")
            # 杀死sshd进程
            cmd = '''ps -ef |grep sshd |grep -v grep |awk '{print $2}' |xargs kill -9'''
            return_code, std_out, std_err = ServiceUtil.execute_cmd(cmd)
            if return_code != "0":
                log.error("Kill sshd command failed.")
                return return_code, std_out, std_err
        # 重启sshd进程
        return_code, std_out, std_err = ServiceUtil.execute_cmd('''/usr/sbin/sshd''')
        if return_code != "0":
            log.error("Restart sshd command failed.")
            return return_code, std_out, std_err
        # 再次查看sshd是否已启动
        return_code, std_out, std_err = ServiceUtil.execute_cmd('''ps -ef |grep sshd | grep -v grep''')
        if return_code != "0":
            log.error("Kill sshd process and restart failed.")
            return return_code, std_out, std_err
        return "0", "", "Kill sshd process and restart successfully."

    @staticmethod
    def change_pwd_ruler(pwd_ruler_config):
        """修改Linux密码规则配置文件：允许重复使用之前的密码"""
        try:
            with open(pwd_ruler_config, "r+") as f:
                config_content = f.read()
                new_config = config_content
                # 检查文件中是否已有use_authtok remember=1
                if not re.search(r"pam_pwhistory\.so\suse_authtok\sremember=1\senforce_for_root", config_content):
                    # 如果没有，则修改，设置use_authtok remember=1
                    repl = "pam_pwhistory.so use_authtok remember=1 enforce_for_root"
                    new_config = re.sub(r"pam_pwhistory\.so\suse_authtok\sremember=\d+\senforce_for_root",
                                        repl, new_config)
                else:
                    # 如果有，则不对文件做写操作
                    log.info(
                        "Password ruler config pwd_ruler_config:[%s] is target. No need to modify" % pwd_ruler_config
                    )
                    return True
                # 将原文件内容清空
                f.seek(0)
                f.truncate()
                # 写入新内容
                f.write(new_config)
            return True
        except Exception as e:
            log.error("Modify password ruler config file error failed")
            return False

    @staticmethod
    def execute_delete_line(user_history_pwd_list, user_name):
        """删除用户历史密码所在行"""
        for line in user_history_pwd_list:
            user = line.split(":")[0]
            if user == user_name:
                user_history_pwd_list.remove(line)

    @staticmethod
    def delete_user_history_pwd(user_name):
        """删除用户历史密码"""
        try:
            with open(SftpConst.PWD_TEMP_SAVE_DATA, "r+") as f:
                user_history_pwd_list = f.readlines()
                ServiceUtil.execute_delete_line(user_history_pwd_list, user_name)
                # 将原文件内容清空
                f.seek(0)
                f.truncate()
                # 写入新内容
                f.writelines(user_history_pwd_list)
            return True
        except Exception as e:
            log.error("Modify password ruler config file error failed")
            return False

    @staticmethod
    def allow_history_pwd(user_name):
        """修改Linux密码规则配置文件：允许用户重复使用之前的密码"""
        # 修改Linux密码规则配置文件
        pwd_ruler_configs = SftpConst.PWD_RULER_SYSTEM_AUTH_LOCAL, SftpConst.PWD_RULER_PASSWORD_AUTH_LOCAL
        for pwd_ruler_config in pwd_ruler_configs:
            # 检查密码规则配置文件是否存在
            if not os.path.exists(pwd_ruler_config):
                # 如果不存在，则退出返回错误
                log.error("Password ruler config file pwd_ruler_config:[%s] does not exist." % pwd_ruler_config)
                return (CustomErrorCode.pwd_ruler_config_not_exist, "",
                        "Password ruler confige file [%s] does not exist." % pwd_ruler_config)
            else:
                # 如果存在，则进行修改
                res = ServiceUtil.change_pwd_ruler(pwd_ruler_config)
                if not res:
                    log.error("Change password ruler config file pwd_ruler_config:[%s] failed." % pwd_ruler_config)
                    return (CustomErrorCode.pwd_ruler_config_modify_failed, "",
                            "Modify password ruler confige file [%s] failed." % pwd_ruler_config)
                log.info("Change password ruler config file pwd_ruler_config:[%s] successfully." % pwd_ruler_config)
        # 删除密码临时保存文件中用户的密码
        if not os.path.exists(SftpConst.PWD_TEMP_SAVE_DATA):
            log.error("User user_name:[%s] history password data file does not exist." % user_name)
            return (CustomErrorCode.user_history_pwd_not_exist, "",
                    "User [%s] history password data file does not exist." % user_name)
        res = ServiceUtil.delete_user_history_pwd(user_name)
        if not res:
            log.error("Delete user user_name:[%s] history password data failed." % user_name)
            return (CustomErrorCode.user_history_pwd_delete_failed, "",
                    "Delete user [%s] history password data failed." % user_name)
        return "0", "", "Change password ruler config file successfully."

    @staticmethod
    def set_pwd(user_name, pwd):
        """设置（或修改）密码"""
        # 修改Linux密码规则配置文件：允许用户重复使用之前的密码
        return_code, std_out, std_err = ServiceUtil.allow_history_pwd(user_name)
        if return_code != "0":
            log.error("Change password ruler config file for user user_name:[%s] failed." % user_name)
            return return_code, std_out, std_err
        log.info("Change password ruler config file for user user_name:[%s] successfully." % user_name)
        # 设置（或修改）密码
        cmd_list = [f"echo '{pwd}'", f"passwd --stdin {user_name}"]
        return_code, std_out, std_err = ServiceUtil.execute_cmd_list(cmd_list)
        if return_code != "0":
            log.error("Set user user_name:[%s] password command failed." % user_name)
            return CustomErrorCode.invalid_os_pwd, std_out, std_err
        return "0", "", "Set user [%s] password command successfully." % user_name

    @staticmethod
    def add_table():
        """
        判断iptables的22端口是否已在白名单，如果不在白名单，则循环执行添加操作（如果添加操作命令执行失败，直接返回False）
        """
        while True:
            return_code1, std_out1, std_err1 = ServiceUtil.execute_cmd(
                'iptables -L -n --line-number |grep "dports 22"')
            return_code2, std_out2, std_err2 = ServiceUtil.execute_cmd(
                'ip6tables -L -n --line-number |grep "dports 22"')
            if std_out1 and std_out2:
                break
            # 不在白名单，执行添加操作
            if not std_out1:
                cmd = '''iptables -A INPUT -p tcp -m multiport --dports 22 -j ACCEPT'''
                return_code, std_out, std_err = ServiceUtil.execute_cmd(cmd)
                if return_code != "0":
                    log.error("Add iptables port 22 to white lists command failed.")
                    return return_code, std_out, std_err
            if not std_out2:
                cmd = '''ip6tables -A INPUT -p tcp -m multiport --dports 22 -j ACCEPT'''
                return_code, std_out, std_err = ServiceUtil.execute_cmd(cmd)
                if return_code != "0":
                    log.error("Add ip6tables port 22 to white lists command failed.")
                    return return_code, std_out, std_err
        return "0", "", "Add iptables port 22 to white lists successfully."

    @staticmethod
    def recursion_chown(user_name, group_name, top):
        """递归设置用户文件的属主"""
        # 用户上传的内容可能很多，为减少日志容量，不能打info日志，仅在error日志中打印关键信息
        for dir_path, dir_names, file_names in os.walk(top):
            try:
                shutil.chown(dir_path, user=user_name, group=group_name)
            except Exception as e:
                log.error("Chown dir_path:%s for user user_name:%s failed." % (dir_path, user_name))
                return False
            for dir_name in dir_names:
                try:
                    shutil.chown(os.path.join(dir_path, dir_name), user=user_name, group=group_name)
                except Exception as e:
                    log.error("Chown dir_name:%s for user user_name:%s failed" %
                              (os.path.join(dir_path, dir_name), user_name))
                    return False
            for file_name in file_names:
                try:
                    shutil.chown(os.path.join(dir_path, file_name), user=user_name, group=group_name)
                except Exception as e:
                    log.error("Chown dir_path:%s for user user_name:%s failed" %
                              (os.path.join(dir_path, file_name), user_name))
                    return False
        return True

    @staticmethod
    def set_owner_of_secondary_directory(user_name, group_name, secondary_path):
        """设置子目录属主，以及设置子目录下的所有除.snapshot文件外的目录及文件的属主"""
        log.info("user_name:%s, group_name:%s, secondary_path:%s" % (user_name, group_name, secondary_path))
        try:
            shutil.chown(secondary_path, user=user_name, group=group_name)
        except Exception as e:
            log.error("Chown secondary_path:%s for user user_name:%s failed"
                      % (secondary_path, user_name))
            return "2", "Chown cmd failed", "Chown (%s) for user (%s) failed" % (secondary_path, user_name)
        return "0", "", "Set owner of user [%s] secondary directory successfully." % user_name


def add_user(user_name, pwd):
    """增加用户"""
    log.info("Adding user user_name:[%s] ...".center(60, "+") % user_name)
    group_name = SftpConst.SFTP_GROUP_NAME
    # 创建用户目录，设置用户目录为用户的家目录，设置家目录权限
    home_path = "/%s/%s" % (group_name, user_name)
    secondary_path = home_path + "/%s" % user_name
    return_code, std_out, std_err = ServiceUtil.make_and_set(home_path, secondary_path, user_name)
    if return_code != "0":
        log.error("Make and set owner and mod of user user_name:%s home directory failed." % user_name)
        # 回滚：删除用户家目录，删除密码临时保存文件中用户的密码，删除用户名
        rollback_return_code, rollback_std_out, rollback_std_err = delete_user(user_name)
        if rollback_return_code != "0":
            return rollback_return_code, rollback_std_out, rollback_std_err
        return return_code, std_out, std_err
    log.info("Make and set owner and mod of user user_name:[%s] home directory successfully." % user_name)

    # 挂载NAS文件系统到用户目录
    device_file_system = "127.0.0.1:/SFTP_FS_e1f0b07c7ad24daaaceba47cda8b4e66/%s" % user_name
    return_code, std_out, std_err = ServiceUtil.execute_cmd('''mount -t nfs -o nosuid,nodev,noexec,vers=3,nolock %s %s'''
                                                            % (device_file_system, secondary_path))
    if return_code != "0":
        log.error("Mount user user_name:%s file system command failed." % user_name)
        # 回滚：删除用户家目录，删除密码临时保存文件中用户的密码，删除用户名
        rollback_return_code, rollback_std_out, rollback_std_err = delete_user(user_name)
        if rollback_return_code != "0":
            return rollback_return_code, rollback_std_out, rollback_std_err
        return return_code, std_out, std_err
    log.info("Mount user user_name:[%s] file system successfully." % user_name)

    # 获取当前user_name 挂载对应的用户id
    time.sleep(5)
    get_u_id_cmd = f"stat -c %u {secondary_path}"
    return_code, std_out, std_err = ServiceUtil.execute_cmd(get_u_id_cmd)
    if return_code != "0":
        log.error(f"Get user {user_name}'s id failed.")
        return return_code, std_out, std_err
    u_id = int(std_out)
    log.info(f"Get {user_name}'s u_id successfully.")

    # 创建组，增加用户
    return_code, std_out, std_err = ServiceUtil.add_group_and_user(group_name, user_name, u_id, home_path)
    if return_code != "0":
        log.error("Add group name group_name:%s and user name user_name:%s failed." % (group_name, user_name))
        return return_code, std_out, std_err
    log.info("Add group name group_name:%s and user name user_name:[%s] successfully." % (group_name, user_name))

    # 设置密码
    return_code, std_out, std_err = ServiceUtil.set_pwd(user_name, pwd)
    if return_code != "0":
        # 回滚：删除密码临时保存文件中用户的密码，删除用户名
        rollback_return_code, rollback_std_out, rollback_std_err = delete_user(user_name)
        release_str_memory(pwd)
        if rollback_return_code != "0":
            return rollback_return_code, rollback_std_out, rollback_std_err
        return return_code, std_out, std_err
    log.info("Set user user_name:[%s] password successfully." % user_name)

    # 设置子目录属主，以及设置子目录下的所有除.snapshot文件外的目录及文件的属主
    return_code, std_out, std_err = ServiceUtil.set_owner_of_secondary_directory(user_name, group_name, secondary_path)
    if return_code != "0":
        log.error("Set owner of user user_name:%s secondary directory failed." % user_name)
        # 回滚：卸载用户目录下挂载的NAS文件系统，删除用户家目录，删除密码临时保存文件中用户的密码，删除用户名
        rollback_return_code, rollback_std_out, rollback_std_err = delete_user(user_name)
        if rollback_return_code != "0":
            return rollback_return_code, rollback_std_out, rollback_std_err
        return return_code, std_out, std_err
    log.info("Set owner of user user_name:[%s] secondary directory successfully." % user_name)
    log.info("Add user user_name:[%s] successfully".center(60, "+") % user_name)
    return "0", "", "Add user [%s] successfully" % user_name


def delete_user(user_name, group_name=SftpConst.SFTP_GROUP_NAME):
    """删除用户"""
    log.info("Deleting user user_name:[%s] ... ".center(60, "+") % user_name)
    # 卸载用户目录下挂载的NAS文件系统
    home_path = "/%s/%s" % (group_name, user_name)
    secondary_path = home_path + "/%s" % user_name
    if os.path.exists(secondary_path):
        # 如果存在子目录，则检查是否存在挂载
        return_code, std_out, std_err = ServiceUtil.execute_cmd('''df -TH | grep %s''' % secondary_path)
        if return_code == "0":
            # 如果存在挂载，则执行卸载
            return_code, std_out, std_err = ServiceUtil.execute_cmd('''umount %s''' % secondary_path)
            if return_code != "0":
                log.error("Umount user user_name:%s file system command failed." % user_name)
                return CustomErrorCode.umount_failed, std_out, std_err
    log.info("Umount user user_name:[%s] file system successfully." % user_name)
    # 删除用户家目录
    time.sleep(5)
    if os.path.exists(home_path):
        return_code, std_out, std_err = ServiceUtil.execute_cmd('''rm -rf %s''' % home_path)
        if return_code != "0":
            log.error("Remove user user_name:%s home directory command failed." % user_name)
            return return_code, std_out, std_err
    log.info("Remove user user_name:[%s] home directory successfully." % user_name)
    # 删除密码临时保存文件中用户的密码
    if os.path.exists(SftpConst.PWD_TEMP_SAVE_DATA):
        res = ServiceUtil.delete_user_history_pwd(user_name)
        if not res:
            log.error("Delete user user_name:%s history password data failed." % user_name)
            return (CustomErrorCode.user_history_pwd_delete_failed, "",
                    "Delete user [%s] history password data failed." % user_name)
    log.info("Delete user user_name:[%s] history password data successfully." % user_name)
    # 删除用户名
    return_code, std_out, std_err = ServiceUtil.list_user_names(SftpConst.SFTP_GROUP_NAME)
    if return_code != "0":
        log.error("List user name user_name:%s command failed." % user_name)
        return return_code, std_out, std_err
    else:
        user_list = std_out
        if user_name in user_list:
            # 用户名存在，则执行删除
            return_code, std_out, std_err = ServiceUtil.execute_cmd('''userdel %s''' % user_name)
            if return_code != "0":
                log.error("Delete user name user_name:%s command failed." % user_name)
                return return_code, std_out, std_err
        else:
            return CustomErrorCode.delete_nonexistent_user, "", "Delete a nonexistent user [%s]." % user_name
    log.info("Delete user name user_name:[%s] successfully." % user_name)
    log.info("Delete user user_name:[%s] successfully".center(60, "+") % user_name)
    return "0", "", "Delete user [%s] successfully" % user_name


def change_pwd(user_name, pwd):
    """修改用户密码"""
    log.info("Changing user user_name:[%s] password... ".center(60, "+") % user_name)
    return_code, std_out, std_err = ServiceUtil.set_pwd(user_name, pwd)
    release_str_memory(pwd)
    if return_code != "0":
        return return_code, std_out, std_err
    log.info("Change user user_name:[%s] password successfully.".center(60, "+") % user_name)
    return "0", "", "Change user [%s] password successfully." % user_name


def start_sftp():
    """启动sftp服务"""
    log.info("Starting sftp service... ".center(60, "+"))
    # 修改/etc/ssh/sshd_config
    if os.path.exists(SftpConst.SSHD_CONFIG):
        res = ServiceUtil.modify_ssh_config()
        if not res:
            log.error("Modify sshd config file failed.")
            return CustomErrorCode.ssh_config_modify_failed, "", "Modify sshd config file failed."
        log.info("Modify sshd config file successfully.")
    else:
        log.error("Sshd config file sshd_config:%s does not exist. Modify failed." % SftpConst.SSHD_CONFIG)
        return CustomErrorCode.ssh_config_not_exist, "", "Sshd config file does not exist. Modify failed."
    # 生成密钥文件
    ssh_key = "%s/ssh_host_rsa_key" % HOST_KEY_PATH, "%s/ssh_host_ecdsa_key" % HOST_KEY_PATH, \
              "%s/ssh_host_ed25519_key" % HOST_KEY_PATH
    ssh_key_type = "rsa", "ecdsa", "ed25519"
    return_code, std_out, std_err = ServiceUtil.generate_ssh_key(ssh_key, ssh_key_type)
    if return_code != "0":
        log.error("Generate ssh key failed.")
        return return_code, std_out, std_err
    log.info("Generate ssh key successfully.")
    # 修改/etc/rsyslog.d/sftpchroot.conf
    return_code, std_out, std_err = ServiceUtil.set_log_config()
    if return_code != "0":
        log.error("Modify syslogd config file failed.")
        return return_code, std_out, std_err
    log.info("Modify syslogd config file successfully.")
    # syslogd进程重启,并查看syslogd是否已成功重启
    return_code, std_out, std_err = ServiceUtil.kill_and_restart_log()
    if return_code != "0":
        log.error("Kill rsyslogd process and restart failed.")
        return return_code, std_out, std_err
    log.info("Kill rsyslogd process and restart successfully.")
    # sshd进程重启, 并查看sshd是否已成功重启
    return_code, std_out, std_err = ServiceUtil.kill_and_restart_ssh()
    if return_code != "0":
        log.error("Kill sshd process and restart failed.")
        return return_code, std_out, std_err
    log.info("Kill sshd process and restart successfully.")
    # 添加iptables的22端口到白名单
    return_code, std_out, std_err = ServiceUtil.add_table()
    if return_code != "0":
        return return_code, std_out, std_err
    log.info("Add iptables port 22 to white lists successfully.")
    log.info("Start sftp service successfully.".center(60, "+"))
    return "0", "", "Start sftp service successfully."


def get_user_info():
    user_list = list()
    try:
        initialize_kmc()
        url = InfraUrlConstant.GET_SECRET + f"?nameSpace={K8sConst.NAMESPACE}&secretName={K8sConst.COMMON_SECRET}"
        response = get_data_from_api(url, "GET")
        if response.status != HttpConst.OK:
            # 访问接口失败
            log.error("Request failed.")
            return False, user_list
        data = json.loads(response.read().decode('utf-8'))
        if data.get("error") is not None:
            log.error("Failed to get the SFTP info.")
            return False, user_list
        log.info("Succeed to get the SFTP info.")
        for items in data.get("data"):
            for key, value in items.items():
                if "inner." not in key:
                    continue
                user_list.append((key.split('.')[1], value))
        return True, user_list
    except Exception:
        log.exception("Exception when get sftp user info.")
        return False, user_list


def service_start():
    """启动服务"""
    log.info("Starting sftp service... ".center(60, "+"))
    # 启动sftp服务
    flag = False
    while not flag:
        return_code, std_out, std_err = start_sftp()
        if return_code != "0":
            log.error("Failed to start SFTP service")
            continue
        flag = True
    # 增加SFTP组
    times = 0
    max_retry = 5
    while times < max_retry:
        group_name = SftpConst.SFTP_GROUP_NAME
        return_code, std_out, std_err = ServiceUtil.add_group_name(group_name)
        if str(return_code) == "0":
            log.info(f"Succeed to add group:{group_name}")
            break
        log.error(f"Failed to add group:{group_name}")
        times += 1
        time.sleep(1)
    # add_list是待添加用户列表，user_list是已保存的用户列表
    add_list = list()
    while True:
        result, user_list = get_user_info()
        if not result:
            log.error("Failed to get user info.")
            time.sleep(30)
            continue
        if not user_list:
            log.info("No sftp user need to add")
            break
        if not add_list:
            add_list.extend(user_list)
        for user_item in user_list:
            if user_item not in add_list:
                # 待添加用户不在common-secret中直接移除
                add_list.remove(user_item)
                continue
            user_name, pwd = user_item[0], user_item[1]
            return_code, std_out, std_err = add_user(user_name, pwd)
            if return_code == "0":
                log.info("Succeed to add SFTP user user_name:%s" % user_name)
                # 添加成功移除
                add_list.remove(user_item)
            else:
                log.error("Failed to add SFTP user user_name:%s" % user_name)
        if not add_list:
            break
        # 添加用户失败，循环添加，为减少日志容量，延长间隔时间
        time.sleep(60)
    log.info("Start sftp service after POD start successfully.".center(60, "+"))
    return "0", "", "Restart sftp service successfully."


def service_manage():
    operation = sys.argv[1]
    if operation == "add_user":
        user_name = sys.argv[2]
        pwd = sys.argv[3]
        res = add_user(user_name, pwd)
        return_sdt_out = "{}|{}|{}".format(res[0], res[1], res[2])
        sys.stdout.write(return_sdt_out)
        return res
    if operation == "delete_user":
        user_name = sys.argv[2]
        res = delete_user(user_name)
        return_sdt_out = "{}|{}|{}".format(res[0], res[1], res[2])
        sys.stdout.write(return_sdt_out)
        return res
    if operation == "change_pwd":
        user_name = sys.argv[2]
        pwd = sys.argv[3]
        res = change_pwd(user_name, pwd)
        return_sdt_out = "{}|{}|{}".format(res[0], res[1], res[2])
        sys.stdout.write(return_sdt_out)
        return res
    if operation == "start":
        res = service_start()
        return_sdt_out = "{}|{}|{}".format(res[0], res[1], res[2])
        sys.stdout.write(return_sdt_out)
        return res


if __name__ == "__main__":
    service_manage()
