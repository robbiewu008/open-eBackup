# coding: utf-8
import os
import shlex
import ssl
import subprocess
import threading
import time

import connexion

from app.common.common import initialize_kmc, kmc_decrypt
from app.common.const import K8sConst, HttpConst, KmcConstant
from app.common.logger import log
from app.service import sftp_management
from app.service.apis.controller import send_backup_response

Time_Interval = 30

CIPHER_SUIT = 'ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256'


def add_user():
    """
    增加SFTP用户名和密码
    :return:
    """
    params = connexion.request.json
    if "username" not in params or "password" not in params:
        return send_backup_response(False, HttpConst.PARAM_ERROR, "params error")
    username = params['username']
    password = params['password']
    flag, std_out, std_err = sftp_management.add_user(username, password)
    if flag == '0':
        log.info(f"Succeed to add SFTP user username:{username}")
        return send_backup_response(True)
    else:
        log.error(f"Failed to add SFTP user username:{username}")
        return send_backup_response(False, flag, std_err)


def delete_user():
    """
    删除SFTP用户
    :return:
    """
    params = connexion.request.json
    if 'username' not in params:
        return send_backup_response(False, HttpConst.PARAM_ERROR, "params error")
    username = params['username']
    flag, std_out, std_err = sftp_management.delete_user(username)
    if flag == '0':
        log.info(f"Succeed to delete SFTP user username:{username}")
        return send_backup_response(True)
    else:
        log.error(f"Failed to delete SFTP user username:{username}")
        return send_backup_response(False, flag, std_err)


def change_pwd():
    """
    修改SFTP用户密码
    :return:
    """
    params = connexion.request.json
    if 'username' not in params or 'password' not in params:
        return send_backup_response(False, HttpConst.PARAM_ERROR, "params error")
    username = params['username']
    password = params['password']
    flag, std_out, std_err = sftp_management.change_pwd(username, password)
    if flag == '0':
        log.info(f"Succeed to change the password of SFTP user username:{username}")
        return send_backup_response(True)
    else:
        log.error(f"Failed to change the password of SFTP user username:{username}")
        return send_backup_response(False, flag, std_err)


def service_status():
    """
    查询SFTP服务状态
    :return:
    """
    # 检查进程
    return_code, std_out, std_err = sftp_management.ServiceUtil.execute_cmd('''ps -ef |grep sshd | grep -v grep''')
    if str(return_code) != "0":
        log.error("The SFTP service is not enabled")
        return send_backup_response(False)
    log.info("The SFTP service has been enabled")
    # 检查端口
    return_code, std_out, std_err = sftp_management.ServiceUtil.execute_cmd('''
    iptables -L -n --line-number |grep "dports 22"''')
    if not std_out:
        log.error("Iptables is not added")
        return send_backup_response(False)
    log.info("Iptables is added")
    return send_backup_response(True)


def run_api_server():
    try:
        initialize_kmc()
        # 获取域名IP地址
        host_ip = os.getenv('POD_IP')
        if not host_ip:
            log.error("host ip is None")
            os._exit(1)
        pod_port = K8sConst.PORT
        # 执行 iptables -C 检查规则是否已存在
        check_command = f"iptables -C INPUT -p tcp --dport {pod_port} -j ACCEPT"
        command_res = subprocess.run(shlex.split(check_command), capture_output=True, text=True)
        if command_res.returncode != 0:
            res = subprocess.run(shlex.split("iptables -A INPUT -p tcp --dport %s -j ACCEPT" % pod_port),
                                 capture_output=True, text=True)
            if str(res.returncode) != '0':
                log.error("Failed to add pod_port:%s to iptables" % pod_port)
                os._exit(1)
        spec_dir = '/opt/sftp/package/src/app/service/apis/'
        options = {"swagger_ui": False}
        api_server = connexion.App(__name__, specification_dir=spec_dir, options=options)
        api_server.add_api('swagger.yaml', strict_validation=True)
        # 创建ssl上下文，创建服务器套接字
        context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
        # 要求客户端提供证书进行验证
        context.verify_mode = ssl.CERT_REQUIRED
        context.load_cert_chain(certfile=KmcConstant.CERT_FILE,
                                keyfile=KmcConstant.KEY_FILE,
                                password=kmc_decrypt(KmcConstant.INFRA_CERT))
        context.load_verify_locations(KmcConstant.CA_FILE)
        # 限制tls版本和加密算法
        context.options |= ssl.OP_NO_TLSv1_1
        context.options |= ssl.OP_NO_TLSv1
        cipher = CIPHER_SUIT
        context.set_ciphers(cipher)
        log.info("Begin to run api server...")
        api_server.run(host=host_ip, port=pod_port, ssl_context=context)
    except Exception:
        log.exception("run api server failed")
        os._exit(1)


def main():
    api_server_job = threading.Thread(target=run_api_server, name="run_api_server")
    api_server_job.start()
    while True:
        time.sleep(Time_Interval)
        log.info("watch thread status")
        watch_thread_list = {'run_api_server': run_api_server}
        current_thread = [thread.name for thread in threading.enumerate()]
        for thread_name in watch_thread_list.keys():
            if thread_name not in current_thread:
                log.error("thread of thread_name:%s not exist, start to create" % thread_name)
                new_name = threading.Thread(target=watch_thread_list.get(thread_name), name=thread_name)
                new_name.start()


if __name__ == '__main__':
    main()
