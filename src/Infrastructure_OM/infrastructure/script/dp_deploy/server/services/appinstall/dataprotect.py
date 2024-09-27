import os
import re
import time
import pexpect
import sys
import paramiko

from server.common.exec_cmd import exec_cmd, exec_cmds_with_pipe, KUBECT_TIMEOUIT, ssh_exec_cmd
from server.common.consts import *
from server.common.logger.logger import logger
from server.schemas.request import InstallSimbaOSRequest, InstallSmartkubeRequest, PreinstallDataProtectRequest, \
    InstallDataProtectRequest, ExpandDataBackupRequest, ExpandSimbaOSRequest, UpgradeDataProtectRequest, \
    DeleteSimbaOSNodeRequest, SynchronizeCertRequest, DataBackupExpandDataBackupRequest, PreUpgradeSimbaOSRequest, \
    UpgradeSimbaOSRequest
from server.common.anonymity import Anonymity
from server.common.exter import exter_attack
from server.services.appinstall import simbaos_upgrade


@exter_attack
def simbaos_preinstall(req: InstallSmartkubeRequest, script_path):
    command = f'sh {script_path} {req.package_name} {req.node_ip}'
    has_issue, stdout, stderr = exec_cmd(command)
    if has_issue:
        logger.error(f'Failed to install smartkube, command is {command}, stdout is {stdout}, stderr is {stderr}.')
        return COMMAND_FAILED, f'Failed to install smartkube, execute out is {stdout}, execute error is {stderr}.'
    logger.info(f'Successfully install smartkube.')
    return COMMAND_SUCCESS, None


@exter_attack
def simbaos_install(req: InstallSimbaOSRequest, script_path):
    try:
        process = pexpect.spawn(
            f'sh {script_path}', encoding='utf-8', timeout=1800)
        process.expect("Please enter the config data in the form of base64: ")
        logger.info(f"sending config")
        process.sendline(req.base64_encoded_config)
        process.expect(pexpect.EOF)
        return_code = process.wait()
        if return_code:
            stdout = process.stdout.read().strip()
            stderr = process.stderr.read().strip()
            logger.error(f'Failed to install simbaOS, execute out:{stdout}, execute error:{stderr}')
            return COMMAND_FAILED, f'Failed to install simbaOS, execute out:{stdout}, execute error:{stderr}'
        logger.info(f'Successfully install simbaOS.')
        return COMMAND_SUCCESS, None
    except pexpect.ExceptionPexpect as e:
        logger.error(f"Install simbaOS error:{Anonymity.process(str(e))}")
        return COMMAND_FAILED, f"Install simbaOS error:{Anonymity.process(str(e))}"


@exter_attack
def simbaos_reset():
    script_path = os.path.join(SCRIPTS_PATH, "simbaos", "reset.sh")
    command = f'sh {script_path}'
    has_issue, stdout, stderr = exec_cmd(command)
    if has_issue:
        logger.error(f'Failed to reset simbaos, command is {command}, stdout is {stdout}, stderr is {stderr}.')
        return COMMAND_FAILED, f'Failed to reset simbaos, execute out is {stdout}, execute error is {stderr}.'
    logger.info(f'Successfully reset simbaos.')
    return COMMAND_SUCCESS, None


@exter_attack
def get_simbaos_status():
    # 后续需要补充：返回node name list
    if os.path.isfile("/var/lib/kubelet/config.yaml"):
        command1 = "kubectl get cm -n kube-system cluster-info-smartkube -ojsonpath={.data.version}"
        command2 = "tail -n 1"
        command3 = "awk {'print $2'}"
        has_issue, version = exec_cmds_with_pipe(command1, command2, command3)
        # out be like "v1.3.0"
        logger.info(f"Simbaos already exist, version is {version}.")

        _, names = exec_cmds_with_pipe(
            'kubectl get nodes',
            "awk {'if (NR>1) print $1'}",
            timeout=KUBECT_TIMEOUIT
        )
        name_list = names.split("\n")
        # ["hn00", ""]
        _, roles = exec_cmds_with_pipe(
            'kubectl get nodes',
            "awk {'if (NR>1) print $3'}",
            timeout=KUBECT_TIMEOUIT
        )
        role_list = ['master' if 'master' in role else 'worker' for role in roles.split("\n")]

        node_role_list = [
            {"name": name, "role": role}
            for name, role in zip(name_list, role_list)
        ]
        return COMMAND_SUCCESS, version, node_role_list
    logger.info(f"Simbaos does not exist.")
    return COMMAND_FAILED, None, None


@exter_attack
def expand_simbaos(req: ExpandSimbaOSRequest):
    try:
        script_path = os.path.join(SCRIPTS_PATH, "simbaos", "expand.sh")
        process = pexpect.spawn(
            f'sh {script_path}', encoding='utf-8', timeout=1800)
        process.expect("Please enter the config data in the form of base64: ")
        process.sendline(req.base64_encoded_config)
        process.expect(pexpect.EOF)
        return_code = process.wait()
        if return_code:
            stdout = process.stdout.read().strip()
            stderr = process.stderr.read().strip()
            logger.error(f'Failed to expand simbaOS, execute out:{stdout}, execute error:{stderr}')
            return COMMAND_FAILED, f'Failed to expand simbaOS, execute out:{stdout}, execute error:{stderr}'
        logger.info(f'Successfully expand simbaOS.')
        return COMMAND_SUCCESS, None
    except pexpect.ExceptionPexpect as e:
        logger.error(f"Expand simbaOS error:{Anonymity.process(str(e))}")
        return COMMAND_FAILED, f"Expand simbaOS error:{Anonymity.process(str(e))}"


@exter_attack
def delete_simbaos_node(req: DeleteSimbaOSNodeRequest):
    script_path = os.path.join(SCRIPTS_PATH, "simbaos", "delete_node.sh")
    command = f'sh {script_path} {req.node_name}'
    has_issue, stdout, stderr = exec_cmd(command)
    if has_issue:
        logger.error(
            f'Failed to delete simbaOS node, node name:{req.node_name}, stdout is {stdout}, stderr is {stderr}.')
        return COMMAND_FAILED, f'Failed to delete simbaOS node, execute out is {stdout}, execute error is {stderr}.'
    logger.info(f'Successfully delete simbaOS node, node name: {req.node_name}.')
    return COMMAND_SUCCESS, None


@exter_attack
def syn_cert(req: SynchronizeCertRequest):
    logger.info(f"Start syn cert")
    host_ip = req.host_ip
    port = 22
    username = req.username
    password = req.password
    src = MICRO_SERVER_CERT_TAR_PATH
    des = MICRO_SERVER_CERT_TAR_PATH

    cmd = (f"tar zcvf {MICRO_SERVER_CERT_TAR_PATH} -C {LOCAL_PV_BASE_PATH} "
           f"comm-data/infrastructure comm-data/protectmanager")
    exec_cmd(cmd)

    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        client.connect(host_ip, port, username, password)
        sftp = client.open_sftp()
        sftp.put(src, des)
        cmd1 = f"tar zxvf {des} -C {LOCAL_PV_BASE_PATH}"
        cmd2 = f"mkdir -p {LOCAL_PV_BASE_PATH}/comm-data/logs"
        cmd3 = f"chown -R 99:99 {LOCAL_PV_BASE_PATH}/comm-data"
        ssh_exec_cmd(client, cmd1)
        ssh_exec_cmd(client, cmd2)
        ssh_exec_cmd(client, cmd3)
        logger.info(f"Successfully syn cert")
    except Exception as e:
        logger.error(f"Failed to syn cert, error: {Anonymity.process(str(e))}")
        return COMMAND_FAILED
    finally:
        time.sleep(5)
        client.close()

    return COMMAND_SUCCESS


@exter_attack
def dataprotect_preinstall(req: PreinstallDataProtectRequest):
    script_path = os.path.join(SCRIPTS_PATH, "dataprotect", "preinstall.sh")
    command = f'sh {script_path} {req.image_package_name}'
    has_issue, stdout, stderr = exec_cmd(command, timeout=DATAPROTECT_INSTALL_WAIT_TIMEOUT)
    if has_issue:
        logger.error(
            f'Failed to load images, image name:{req.image_package_name}, stdout is {stdout}, stderr is {stderr}.')
        return COMMAND_FAILED, f'Failed to load images, execute out is {stdout}, execute error is {stderr}.'
    logger.info(f'Successfully load images, image name: {req.image_package_name}.')
    return COMMAND_SUCCESS, None


@exter_attack
def dataprotect_install(req: InstallDataProtectRequest, script_path):
    command = f'sh {script_path} {req.chart_package_name} {req.pm_replicas} {req.pe_replicas}'
    has_issue, stdout, stderr = exec_cmd(command, timeout=DATAPROTECT_INSTALL_WAIT_TIMEOUT)
    if has_issue:
        logger.error(
            f'Failed to install dataprotect, chart name:{req.chart_package_name}, '
            f'stdout is {stdout}, stderr is {stderr}.')
        return COMMAND_FAILED, (f'Failed to install dataprotect, chart name:{req.chart_package_name}, '
                                f'execute out is {stdout}, execute error is {stderr}.')
    logger.info(f'Successfully install dataprotect, chart name: {req.chart_package_name}.')
    return COMMAND_SUCCESS, None


@exter_attack
def dataprotect_reset():
    script_path = os.path.join(SCRIPTS_PATH, "dataprotect", "reset.sh")
    command = f'sh {script_path}'
    has_issue, stdout, stderr = exec_cmd(command)
    if has_issue:
        logger.error(
            f'Failed to reset dataprotect, stdout is {stdout}, stderr is {stderr}.')
        return COMMAND_FAILED, f'Failed to reset dataprotect, execute out is {stdout}, execute error is {stderr}.'
    logger.info(f'Successfully reset dataprotect.')
    return COMMAND_SUCCESS, None


@exter_attack
def get_dataprotect_status():
    command1 = "sudo helm list -Aa"
    command2 = "grep dataprotect"
    command3 = "awk '{print $8, $9}'"
    has_issue, out = exec_cmds_with_pipe(command1, command2, command3)
    if has_issue:
        logger.error(f"Failed to get dataprotect status, stderr is {out}.")
        return COMMAND_WRONG, None, None, f"Failed to get dataprotect status, stderr is {out}."
    if not out:
        logger.info(f"No dataprotect.")
        return COMMAND_SUCCESS, None, None, f"No dataprotect."
    match = re.search(r'\b(\w+)\s+(\S+)', out)
    status = match.group(1)
    version = match.group(2)
    if status != "deployed":
        logger.info(f"Get dataprotect, but the status is {status}.")
        return COMMAND_FAILED, status, None, f"Get dataprotect, but the status is {status}."
    logger.info(f"dataprotect already deployed.")
    return COMMAND_SUCCESS, status, version, f"dataprotect already deployed."


@exter_attack
def expand_dataprotect(req: ExpandDataBackupRequest, script_path):
    command = f'sh {script_path} {req.chart_package_name} {req.master_replicas} {req.worker_replicas}'
    has_issue, stdout, stderr = exec_cmd(command, timeout=DATAPROTECT_INSTALL_WAIT_TIMEOUT)
    if has_issue:
        return COMMAND_FAILED, f"Expand dataprotect failed, stdout is {stdout}, stderr is {stderr}."
    return COMMAND_SUCCESS, f"Expand dataprotect successfully."


@exter_attack
def databackup_expand_dataprotect(req: DataBackupExpandDataBackupRequest, script_path):
    command = f'sh {script_path} {req.chart_package_name} {req.replicas} {req.node1} {req.node2} {req.node3}'
    has_issue, stdout, stderr = exec_cmd(command, timeout=DATAPROTECT_INSTALL_WAIT_TIMEOUT)
    if has_issue:
        return COMMAND_FAILED, f"Expand dataprotect failed, stdout is {stdout}, stderr is {stderr}."
    return COMMAND_SUCCESS, f"Expand dataprotect successfully."


@exter_attack
def upgrade_dataprotect(req: UpgradeDataProtectRequest):
    cmd = (f"sudo helm upgrade dataprotect {req.chart_package_name} --set global.master_replicas={req.master_replicas} "
           f"--set global.worker_replicas={req.worker_replicas} --set global.deploy_type=d7")
    has_issue, stdout, stderr = exec_cmd(cmd)
    if has_issue:
        return COMMAND_FAILED, f"Upgrade dataprotect failed, stdout is {stdout}, stderr is {stderr}."
    return COMMAND_SUCCESS, f"Upgrade dataprotect successfully."


@exter_attack
def pre_upgrade_simbaos(req: PreUpgradeSimbaOSRequest):
    try:
        simbaos_upgrade.pre_upgrade(req.package_name, req.node_ip, req.cert_type)
    except Exception as e:
        return COMMAND_FAILED, f"Pre upgrade simbaos failed, stdout is {Anonymity.process(str(e))}."
    return COMMAND_SUCCESS, f"Pre upgrade simbaos successfully."


@exter_attack
def upgrade_simbaos(req: UpgradeSimbaOSRequest):
    try:
        simbaos_upgrade.upgrade_simbaos(req.cert_type)
    except Exception as e:
        return COMMAND_FAILED, f"Upgrade simbaos failed, stdout is {e}."
    return COMMAND_SUCCESS, f"Upgrade simbaos successfully."


@exter_attack
def post_upgrade_simbaos():
    try:
        simbaos_upgrade.post_upgrade()
    except Exception as e:
        return COMMAND_FAILED, f"Post upgrade simbaos failed, stdout is {Anonymity.process(str(e))}."
    return COMMAND_SUCCESS, f"Post upgrade simbaos successfully."

