import os.path
import ssl
import shutil
import uvicorn
import argparse
import time
import subprocess
import hashlib
import signal
import multiprocessing as mp
from fastapi import FastAPI, Depends
import pexpect
import sys
import warnings
from urllib3.exceptions import SubjectAltNameWarning, InsecureRequestWarning
from server.common import mount
from server.api.rest.routers import register
from server.common.logger.logger import logger
from server.common.exec_cmd import exec_cmds_with_pipe
from server.common import consts
from server.common.local_kmc import decrypt_private_key_passwd
from server.services.underlay.nfs_share import verify_token

dpserver_process_handler = None
mount_process_handler = None
api = None
# 启动标志位：用于证书是否更新判断
pacific_cert_sha256 = 0
nfs_share_cert_sha256 = 0


def sigterm_handler(signum, frame):
    logger.info("detect terminate, start to handle")
    global mount_process_handler
    global dpserver_process_handler
    if mount_process_handler:
        logger.info("mount process handler working")
        mount_process_handler.terminate()
        mount_process_handler.join()
        logger.info("resolved mount handler")
    if dpserver_process_handler:
        logger.info("dpserver process handler working")
        dpserver_process_handler.terminate()
        dpserver_process_handler.join()
        logger.info("resolved dpserver process handler")
    sys.exit(0)


def get_key_pass(key_path):
    key_pass = decrypt_private_key_passwd(key_path)
    return key_pass.strip()


def create_temp_ssl(cert_path):
    # 1. 解析私钥密码
    logger.info("Start to decrypt private password.")
    key_passwd_path = consts.SSL_KEYFILE_PASSWORD_UNDECRYPT
    decrypt_pass = get_key_pass(key_passwd_path)
    logger.info("Successfully decrypt private password.")

    # 2. 转换private key的格式至pem格式
    logger.info("Start revert type .key to .pem")
    p = pexpect.spawn(
        f'openssl rsa -in {consts.SSL_KEYFILE_PATH} '
        f'-out {consts.SSL_KEYFILE_PATH_DPSERVER}', encoding='utf-8',
        env={"LD_LIBRARY_PATH": ''})
    p.expect(f"Enter pass phrase for {consts.SSL_KEYFILE_PATH}:")
    p.sendline(decrypt_pass)
    p.expect(pexpect.EOF)
    p.wait()
    logger.info("Successfully revert type .key to .pem")

    # 3. 生成服务端证书和CA证书
    logger.info("Start generating server&CA certificates by clipping.")
    cmd2 = f"sed -n '0,/END CERTIFICATE/p' {consts.SSL_CERTFILE_PATH}"
    cmd2_2 = f"tee {consts.SSL_CERTFILE_PATH_DPSERVER}"
    _ = exec_cmds_with_pipe(cmd2, cmd2_2)
    cmd3 = f"sed '0,/END CERTIFICATE/d' {consts.SSL_CERTFILE_PATH}"
    cmd3_2 = f"tee {consts.SSL_CA_CERTFLE_PATH_DPSERVER}"
    has_issue, out = exec_cmds_with_pipe(cmd3, cmd3_2)
    if has_issue:
        logger.error(f"Generate server cert and ca cert failed, out is{out}")
        return consts.COMMAND_FAILED, None
    logger.info("Successfully generating server&CA certificates.")
    return consts.COMMAND_SUCCESS, decrypt_pass


def run_uvicorn(address, decrypt_pass):
    uvicorn.run(
        api,
        host=address,
        port=consts.DPSERVER_PORT,
        ssl_keyfile=consts.SSL_KEYFILE_PATH_DPSERVER,
        ssl_certfile=consts.SSL_CERTFILE_PATH_DPSERVER,
        ssl_ca_certs=consts.SSL_CA_CERTFLE_PATH_DPSERVER,
        ssl_keyfile_password=decrypt_pass,
        ssl_ciphers=consts.SSL_CIPHER
    )


def init_dpserver(address, cert_path):
    signal.signal(signal.SIGTERM, signal.SIG_DFL)
    _, decrypt_pass = create_temp_ssl(cert_path)
    if not decrypt_pass:
        logger.error("Create temp ssl failed.")
        return
    logger.info("Create temp ssl success.")
    run_uvicorn(address, decrypt_pass)


def is_mount_cert_ok():
    return os.path.ismount('/opt/DataBackup')


def sha256_of_dir(
    path,
    key_files: [str] = None,
):
    if os.path.isdir(path):
        sha256 = hashlib.sha256()
        for root, dirs, files in os.walk(path):
            files.sort()
            for filename in files:
                filepath = os.path.join(root, filename)
                file_relpath = os.path.relpath(filepath, path)
                if key_files is not None and file_relpath not in key_files:
                    continue
                with open(filepath, 'rb') as f:
                    sha256.update(f.read())
        return sha256.hexdigest()
    return None


def is_master():
    return os.path.isdir(consts.PACIFIC_CERT)


def is_dpserver_running() -> bool:
    return dpserver_process_handler is not None and dpserver_process_handler.is_alive()


def restart_dpserver(address, cert_path):
    global dpserver_process_handler
    if dpserver_process_handler is not None:
        dpserver_process_handler.kill()
        dpserver_process_handler.join()
    dpserver_process_handler = mp.Process(
        target=init_dpserver,
        args=(address,),
        kwargs={"cert_path": cert_path}
    )
    dpserver_process_handler.start()


def sync_dir(src, dest, key_files: [str] = None, sync_files: [str] = None) -> (bool, bool):
    """同步src和dest
    Args:
        key_files:  用来计算文件夹src和dest指纹sha256的文件
        sync_files: 如果src和dest指纹不相同, 需要同步的文件
    Returns:
        first: 返回src和dest是否已经同步
        second: 返回是否发生了从src到dest的文件拷贝
    """
    try:
        if not os.path.isdir(src):
            return False, False
        src_hash = sha256_of_dir(src, key_files)
        dest_hash = sha256_of_dir(dest, key_files)
        if src_hash is not None and dest_hash is not None and src_hash == dest_hash:
            return True, False

        if os.path.exists(dest):
            shutil.rmtree(dest)
        os.makedirs(dest, exist_ok=True)

        if sync_files is None:
            cmd = subprocess.run(f'cp -rfT {src} {dest}', shell=True)
            cmd.check_returncode()
        else:
            for file in sync_files:
                cmd = subprocess.run(f'cp -rfT {src}/{file} {dest}/{file}', shell=True)
                cmd.check_returncode()
        logger.info(f"Successfully synced {src} to {dest}")
        return True, True
    except Exception as e:
        logger.warning(f"Unable to sync directory {src} to {dest}, {e}")
        return False, False


def sync_dir_kmc(src, dest):
    return sync_dir(
        src, dest,
        key_files=[consts.KMC_LIB_NAME],
        sync_files=[consts.KMC_LIB_NAME]
    )


def manager_dpserver_loop(address):
    cert_synced, cert_changed = sync_dir(
        src=consts.PACIFIC_CERT,
        dest=consts.DPSERVER_CERT
    )
    kmc_synced, kmc_changed = sync_dir_kmc(
        src=consts.PACIFIC_KMC,
        dest=consts.DPSERVER_KMC
    )
    if not (cert_synced and kmc_synced):
        logger.warning('Failed to sync cert or kmc')
    elif not is_dpserver_running() or cert_changed or kmc_changed:
        restart_dpserver(address, consts.DPSERVER_CERT)

    if is_mount_cert_ok():
        sync_dir(
            src=consts.DPSERVER_CERT,
            dest=consts.MOUNT_CERT,
            key_files=[os.path.basename(consts.SSL_CERTFILE_PATH)]
        )
        sync_dir_kmc(consts.DPSERVER_KMC, consts.MOUNT_KMC)


def worker_dpserver_loop(address):
    if is_mount_cert_ok():
        cert_synced, cert_changed = sync_dir(
            src=consts.MOUNT_CERT,
            dest=consts.DPSERVER_CERT
        )
        kmc_synced, kmc_changed = sync_dir_kmc(
            src=consts.MOUNT_KMC,
            dest=consts.DPSERVER_KMC
        )
        if not (cert_synced and kmc_synced):
            logger.warning('Failed to sync cert or kmc')
        elif not is_dpserver_running() or kmc_changed or cert_changed:
            restart_dpserver(address, consts.DPSERVER_CERT)


def dpserver_main(address):
    while True:
        if is_master():
            manager_dpserver_loop(address)
        else:
            worker_dpserver_loop(address)
        time.sleep(consts.SLEEP_TIME)


def e6000_main(args):
    global mount_process_handler
    global api
    # get hostname and do mount circularly
    mount_process_handler = mp.Process(target=mount.mount_main)
    mount_process_handler.start()

    # cgroup setting
    logger.info("Start setting cgroup")
    cgroup_script_path = os.path.join(consts.SCRIPTS_PATH, "cgroups.sh")
    p_cgroup = subprocess.Popen(['sh', cgroup_script_path])
    _, cgroup_err = p_cgroup.communicate()
    if p_cgroup.returncode:
        logger.error(f"Create cgroup error: {cgroup_err}")
        # sys.exit(1)

    # firewall setting
    logger.info("Open firewall port")
    open_port_path = os.path.join(consts.SCRIPTS_PATH, "add_firewall_ports.sh")
    logger.info(f"open_port_path is {open_port_path}")

    cmd = subprocess.run(
        f'/bin/sh {open_port_path}',
        env={'PYTHONHOME': '/usr'},
        shell=True,
        capture_output=True,
    )
    if cmd.returncode != 0:
        logger.warning("Open firewall port failed, "
                       f"out: {cmd.stdout}, error:{cmd.stderr}")

    # register FastAPI api
    warnings.simplefilter('ignore', SubjectAltNameWarning)
    warnings.simplefilter('ignore', InsecureRequestWarning)
    api = FastAPI(dependencies=[Depends(verify_token)])
    register(api)

    # 注册SIGTERM：用于systemctl stop时，kill 子进程。
    signal.signal(signal.SIGTERM, sigterm_handler)

    # open dpserver service
    dpserver_main(args.address)
