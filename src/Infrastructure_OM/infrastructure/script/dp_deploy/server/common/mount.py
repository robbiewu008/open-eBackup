import subprocess as sp
import os
import socket
import time
import signal
from server.common.logger.logger import logger
from server.common import consts
from server.common.exec_cmd import exec_cmd
import shutil


CMD_TIMEOUT = 10


def check_mount(*paths):
    for path in paths:
        cmd = sp.run(f'mountpoint -q {path}', timeout=CMD_TIMEOUT, shell=True)
        if cmd.returncode != 0:
            return False
    return True


def umount(*paths):
    for path in paths:
        sp.run(f'umount -l {path}', timeout=CMD_TIMEOUT, shell=True)


def mount(path, namespace):
    umount(path)
    r = sp.run(
        f'mount -t nfs -o nosuid,nodev,noexec,nolock,soft 127.0.0.1:/{namespace} {path}',
        timeout=CMD_TIMEOUT,
        shell=True
    )
    r.check_returncode()


def change_owner_group(path, username, groupname):
    shutil.chown(path, username, groupname)


def mount_databackup():
    mount('/opt/DataBackup', 'global_image_repo')
    os.chmod('/opt/DataBackup', 750)
    change_owner_group('/opt/DataBackup', 'dpserver', 'dpserver')
    os.makedirs('/opt/DataBackup/packages', exist_ok=True)
    logger.info('Successfully mounted databackup nfs')


def mount_simbaos_nfs(path, namespace, hostname):
    mount(path, namespace)
    os.makedirs(os.path.join(path, hostname), exist_ok=True)
    umount(path, namespace)
    mount(path, f'{namespace}/{hostname}')


def mount_simbaos(hostname):
    umount('/opt/k8s/SimbaOS', '/opt/k8s/log', '/opt/k8s/db')
    mount_simbaos_nfs('/opt/k8s/SimbaOS', 'nfs_simbaos', hostname)
    mount_simbaos_nfs('/opt/k8s/db', 'nfs_simbaos_db', hostname)
    mount_simbaos_nfs('/opt/k8s/log', 'nfs_simbaos_log', hostname)


def mount_loop():
    while True:
        try:
            if not check_mount('/opt/DataBackup'):
                logger.info('Start to mount DataBackup')
                mount_databackup()
                logger.info('Successfully mount DataBackup')
            if not check_mount('/opt/k8s/SimbaOS', '/opt/k8s/log', '/opt/k8s/db'):
                logger.info('Start to mount SimbaOS nfs')
                hostname = socket.gethostname()
                mount_simbaos(hostname)
                if os.path.isfile("/etc/kubernetes/kubelet.conf"):
                    cmd = "sh /opt/k8s/run/scripts/cluster.sh start_base_component"
                    has_issue, stdout, stderr = exec_cmd(cmd)
                    if has_issue:
                        logger.error(f"Failed to restart SimbaOS service, stdout is {stdout}, stderr is {stderr}")
                logger.info('Successfully mount SimbaOS nfs')
        except Exception as e:
            logger.error(f'Failed to mount databackup and simbaos, {e}')
        time.sleep(consts.SLEEP_TIME)


def mount_main(reset: bool = False):
    dirs = [
        '/opt/DataBackup',
        '/opt/k8s/db',
        '/opt/k8s/log',
        '/opt/k8s/SimbaOS'
    ]
    if not reset:
        signal.signal(signal.SIGTERM, signal.SIG_DFL)
        for d in dirs:
            if not check_mount(d):  # 避免失效的挂载路径导致makedirs返回错误
                umount(d)
            os.makedirs(d, exist_ok=True)
        # mount
        logger.info('Start to main mount loop')
        mount_loop()
    else:
        logger.info('Start to reset all mount points')
        umount(*dirs)
        logger.info('Successfully reset all mount points')
