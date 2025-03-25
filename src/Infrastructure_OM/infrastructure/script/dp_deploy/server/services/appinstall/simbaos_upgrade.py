import tarfile
import subprocess as sp
import json
import shutil
import os
import time

from server.common.consts import PACKAGE_PATH
from server.services.package_manage.package import get_basename
from server.common.logger.logger import logger


SIMBAOS_BASE_PATH = '/opt/k8s/SimbaOS'
SIMBAOS_PACKAGE_PATH = f'{SIMBAOS_BASE_PATH}/package'
SIMBAOS_UPGRADE_PACKAGE_PATH = f'{SIMBAOS_BASE_PATH}/upgrade'
SIMBAOS_BACKUP_PACKAGE_PATH = f'{SIMBAOS_BASE_PATH}/backup'

SIMBAOS_USER = "ContainerOSUser"
SIMBAOS_GROUP = "ContainerOSGroup"

SIMBAOS_UPGRADE_STATUS_RUNNING = 'running'
SIMBAOS_UPGRADE_STATUS_FAILED = 'failed'
SIMBAOS_UPGRADE_STATUS_SUCCEED = 'succeed'

# 检测超时时间设置为20分钟
DETECT_TIMEOUT = 20*60

COMMAND_SUCCESS = 0
COMMAND_FAILED = 1
COMMAND_COMPLETE = 2


class UpgradeException(Exception):
    def __init__(self, message: str):
        self.message = message
        super().__init__(self.message)


def extract_tar_gz(file_path, extract_to):
    with tarfile.open(file_path, 'r:gz') as tar:
        tar.extractall(path=extract_to)


def exec_cmd(cmd) -> str:
    result = sp.run(cmd, shell=True, capture_output=True)
    result.check_returncode()
    return result.stdout.decode().strip()


def exec_cmd_without_check_return(cmd) -> str:
    result = sp.run(cmd, shell=True, capture_output=True)
    return result.stdout.decode().strip()


def rollback_simbaos(cert_type):
    exec_cmd(f'smartkube upgrade rollback --oriPackagePath={SIMBAOS_PACKAGE_PATH} --format-print '
             f'--folder={SIMBAOS_BACKUP_PACKAGE_PATH} --packagePath={SIMBAOS_UPGRADE_PACKAGE_PATH} '
             f'--certType={cert_type}')


def upgrade_simbaos_precheck(cert_type='pacific'):
    try:
        r = exec_cmd_without_check_return(f'/usr/bin/smartkube upgrade precheck --certType={cert_type} --format-print')
        if r == "upgrade precheck ok":
            return COMMAND_COMPLETE
        else:
            return COMMAND_SUCCESS
    except Exception as e:
        logger.info(f"upgrade precheck failed, {e}")
        return COMMAND_SUCCESS


def upgrade_simbaos_backup(cert_type='pacific'):
    if cert_type == "pacific":
        exec_cmd(
            'smartkube upgrade backup '
            f'--folder={SIMBAOS_BACKUP_PACKAGE_PATH} '
            f'--certType={cert_type}'
        )
    else:
        exec_cmd(
            'smartkube upgrade backup '
            f'--folder={SIMBAOS_BACKUP_PACKAGE_PATH} '
        )


def upgrade_simbaos_getbatch() -> [[str]]:
    result = exec_cmd('smartkube upgrade getbatch')
    batches = result.split(';')
    return [batch.split(',') for batch in batches]


def cycle_detect_status(detect_fun, check_interval_secs=10, **kwargs):
    timeout_retries_cnt = DETECT_TIMEOUT / check_interval_secs
    retries = 0
    while retries < timeout_retries_cnt:
        retries += 1
        r = detect_fun(**kwargs)
        if r == COMMAND_COMPLETE:
            return
        time.sleep(check_interval_secs)
    raise Exception(f"status not normal")


def check_upgrade_status(nodes, cert_type):
    if cert_type == "pacific":
        r = exec_cmd(f'smartkube upgrade status --certType={cert_type}')
    else:
        r = exec_cmd(f'smartkube upgrade status')
    if not r:
        return COMMAND_SUCCESS
    node_status = json.loads(r)['nodes']
    try:
        if all(node_status[node]['status'] == SIMBAOS_UPGRADE_STATUS_SUCCEED
               for node in nodes):
            return COMMAND_COMPLETE
    except KeyError as e:
        return COMMAND_SUCCESS

    failed_nodes = [
        node for node in nodes
        if node_status[node] == SIMBAOS_UPGRADE_STATUS_FAILED
    ]
    if len(failed_nodes) > 0:
        raise UpgradeException(f"Failed to upgrade simbaos at {failed_nodes}")

    return COMMAND_SUCCESS


def upgrade_simbaos_batch(nodes: [str], is_init_batch: bool, cert_type: str):
    batch = ','.join(nodes)
    init_batch_args = '--init' if is_init_batch else ''
    exec_cmd(
        'smartkube upgrade node '
        f'--nodes={batch} '
        f'{init_batch_args} '
        f'--oriPackagePath={SIMBAOS_PACKAGE_PATH} '
        f'--packagePath={SIMBAOS_UPGRADE_PACKAGE_PATH}'
    )

    cycle_detect_status(check_upgrade_status, nodes=nodes, cert_type=cert_type)


def upgrade_simbaos_postcheck(cert_type):
    cycle_detect_status(check_simbaos_status, cert_type=cert_type)


def upgrade_precheck(cert_type):
    cycle_detect_status(upgrade_simbaos_precheck, cert_type=cert_type)


def check_simbaos_status(cert_type):
    if cert_type == "pacific":
        r = exec_cmd_without_check_return(f'smartkube upgrade postcheck --certType={cert_type} --format-print')
    else:
        r = exec_cmd_without_check_return(f'smartkube upgrade postcheck --format-print')
    if r == "upgrade postcheck ok":
        return COMMAND_COMPLETE
    else:
        return COMMAND_SUCCESS


def upgrade_simbaos_plugin():
    exec_cmd(
        'smartkube upgrade plugin '
        '--ignore-addon-phase='
        '"DHAC component deployment,Monitor component deployment,App-manager component deployment" '
        f'--folder={SIMBAOS_UPGRADE_PACKAGE_PATH}'
    )


def upgrade_simbaos_postupgrade():
    exec_cmd('smartkube upgrade postupgrade --format-print')


def do_process_with_rollback(fun, roll_back_cert_type="pacific", **kwargs):
    try:
        fun(**kwargs)
    except Exception as e:
        rollback_simbaos(roll_back_cert_type)
        raise e


def upgrade_simbaos(cert_type):
    # 升级前检查
    logger.info("Start to precheck Simbaos status")
    upgrade_precheck(cert_type)
    # 升级前备份
    logger.info("Start to backup Simbaos status")
    upgrade_simbaos_backup(cert_type)
    # 获取升级批次信息
    logger.info("Get batch to upgrade")
    batches = upgrade_simbaos_getbatch()

    # 分批升级
    logger.info("Start to upgrade Simbaos")
    is_init_batch = True
    for batch in batches:
        upgrade_simbaos_batch(batch, is_init_batch, cert_type)
        do_process_with_rollback(upgrade_simbaos_batch,
                                 roll_back_cert_type=cert_type,
                                 nodes=batch,
                                 is_init_batch=is_init_batch,
                                 cert_type=cert_type)

    # 升级插件
    logger.info("Start to upgrade Simbaos plugin")
    do_process_with_rollback(upgrade_simbaos_plugin,
                             roll_back_cert_type=cert_type)

    # 升级后检查
    logger.info("Start to postcheck of Simbaos upgrade")
    do_process_with_rollback(upgrade_simbaos_postcheck,
                             roll_back_cert_type=cert_type,
                             cert_type=cert_type)

    # 清理升级状态
    logger.info("Clean Simbaos upgrade status")
    upgrade_simbaos_postupgrade()


def copy_with_overwrite(source_path, destination_path):
    if os.path.exists(destination_path):
        shutil.rmtree(destination_path)

    shutil.copytree(source_path, destination_path)


def copy_file_with_overwrite(source, destination):
    if os.path.exists(destination):
        if os.path.isfile(destination):
            os.remove(destination)
        else:
            shutil.rmtree(destination)
    shutil.copy2(source, destination)


def pre_upgrade(upgrade_package_name, node_ip, cert_type):
    # 包轮转
    logger.info("Start to pre upgrade simbaos")
    upgrade_package_name = get_basename(upgrade_package_name)
    source_path = os.path.join(PACKAGE_PATH, upgrade_package_name, "SimbaOS")
    copy_with_overwrite(source_path, SIMBAOS_UPGRADE_PACKAGE_PATH)

    source_path = os.path.join(SIMBAOS_UPGRADE_PACKAGE_PATH, "action", "smartkube")
    copy_file_with_overwrite(source_path, "/usr/bin/smartkube")

    if cert_type == "pacific":
        exec_cmd(f'smartkube preinstall agent --nodeIP={node_ip} '
                f'--deployType=1 --certType={cert_type} --packagePath={SIMBAOS_UPGRADE_PACKAGE_PATH}')
    else:
        exec_cmd(f'smartkube preinstall agent --nodeIP={node_ip} '
                f' --packagePath={SIMBAOS_UPGRADE_PACKAGE_PATH}')


    # 权限更新
    os.makedirs(SIMBAOS_BACKUP_PACKAGE_PATH, exist_ok=True)
    exec_cmd(f'chown -R {SIMBAOS_USER}:{SIMBAOS_GROUP} {SIMBAOS_BACKUP_PACKAGE_PATH}')
    exec_cmd(f'chown -R {SIMBAOS_USER}:{SIMBAOS_GROUP} {SIMBAOS_UPGRADE_PACKAGE_PATH}')
    exec_cmd(f'chown -R {SIMBAOS_USER}:{SIMBAOS_GROUP} {SIMBAOS_PACKAGE_PATH}')


def post_upgrade():
    # 包轮转
    logger.info("Start to post upgrade simbaos")
    copy_with_overwrite(SIMBAOS_UPGRADE_PACKAGE_PATH, SIMBAOS_PACKAGE_PATH)
