import os
import sys
import time
import yaml
import subprocess
from server.common.exec_cmd import exec_cmd
from server.common.consts import DPSERVER_BACKUP_PATH, DPSERVER_PATH, PACKAGE_PATH, COMMAND_FAILED
from server.schemas.request import RequestUpgradeDpserver
from server.services.package_manage.package import get_basename
from server.common.logger.logger import logger
from server.common.exter import exter_attack
from server.services.package_manage.package import verify_pkg

@exter_attack
def get_dpserver_version():
    version_yaml_path = os.path.join(DPSERVER_PATH, "manifest.yml")
    with open(version_yaml_path, 'r') as file:
        yaml_content = yaml.safe_load(file)
    version = yaml_content.get("Version")
    return version


@exter_attack
def upgrade_dpserver(req: RequestUpgradeDpserver):
    os.makedirs(DPSERVER_BACKUP_PATH, exist_ok=True)
    command = f"cp -rfT {DPSERVER_PATH} {DPSERVER_BACKUP_PATH}"
    _ = exec_cmd(command)

    upgrade_package_basename = get_basename(req.upgrade_package_name)
    logger.info(f"upgrade package basename is {upgrade_package_basename}")
    script_path = os.path.join(PACKAGE_PATH, upgrade_package_basename, "dpserver", "dp_install.sh")
    logger.info(f"script path is {script_path}")
    # dpserver升级脚本特殊性：1. 本身位于共享nfs里，2. 需要root权限执行
    # 针对上述特殊性：需要防止admin用户被窃后，更换升级脚本，并用root执行，高权限破坏系统。
    # 执行命令前，需重新校验、解压
    package_path = os.path.join(PACKAGE_PATH, req.upgrade_package_name)
    base_path = os.path.join(PACKAGE_PATH, upgrade_package_basename)
    has_issue = verify_pkg(package_path, base_path)
    if has_issue:
        logger.error(f"Upgrade dpserver failed, package verified failed!")
        return COMMAND_FAILED
    command = ["tar", "-zxvf", package_path, "-C", base_path]
    has_issue, stdout, stderr = exec_cmd(command)
    if has_issue:
        logger.error(
            f"Failed to unpack dpserver package. package path:{package_path}, dir:{base_path}, stdout:{stdout}, stderr:{stderr}")
        return COMMAND_FAILED
    p = subprocess.Popen(['sh', script_path, 'upgrade', upgrade_package_basename])
