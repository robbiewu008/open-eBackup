import os
import subprocess
import tarfile
import hashlib
from fastapi import UploadFile
from server.common.exec_cmd import exec_cmd, exec_cmds_with_pipe
from server.common.consts import HOME_PACKAGE_PATH, PACKAGE_PATH, MOUNT_PATH
from server.schemas.request import MoveFileRequest
from server.common.consts import LOG_PATH, CHECK_SUM_FILE_NAME, CRL_FILE_NAME, CBB_VERIFY_TOOL, CMS_FILE_NAME, \
    VERIFY_CHECKSUM_CMD, MAX_SINGLE_UPLOAD, COMMAND_SUCCESS, COMMAND_FAILED, EXTENSION_LIST, VERIFY_TOOL_LIST
from server.common.logger.logger import logger
from server.common.anonymity import Anonymity
from server.common.exter import exter_attack
from server.common.calculator_sha256 import opened_file_sha256sum

COMPONENT_NAME = os.path.basename(__file__)


def calculate_file_sha256(file_path):
    sha256_hash = hashlib.sha256()
    with open(file_path, "rb") as file:
        for byte_block in iter(lambda: file.read(4096), b""):
            sha256_hash.update(byte_block)
    hash_value = sha256_hash.hexdigest()
    return hash_value


@exter_attack
def upload(package_size: str, file: UploadFile):
    file_path = os.path.join(HOME_PACKAGE_PATH, file.filename)
    if has_slash(file.filename):
        return COMMAND_FAILED

    os.makedirs(os.path.dirname(file_path), exist_ok=True)
    logger.info(f"Start upload file. file_path:{file_path}")
    try:
        with open(file_path, "wb") as file_object:
            for chunk in iter(lambda: file.file.read(MAX_SINGLE_UPLOAD), b""):
                file_object.write(chunk)
    except Exception as e:
        logger.error(f"Failed to upload file, error:{Anonymity.process(str(e))}.")
        return COMMAND_FAILED
    logger.info("Successfully upload file.")

    destination_path = os.path.join(PACKAGE_PATH, file.filename)
    os.makedirs(os.path.dirname(destination_path), exist_ok=True)

    # 判断是否挂载点是否存在，若不存在，需要删除。
    command = f"mountpoint -q {MOUNT_PATH}"
    has_issue, stdout, stderr = exec_cmd(command)
    if has_issue:
        logger.error(f"{MOUNT_PATH} does not exist! out is {stdout}, err is {stderr}.")
        os.remove(file_path)
        return COMMAND_FAILED

    # 转移包
    logger.info("Start move file.")
    command = ["mv", "-f", file_path, destination_path]
    has_issue, stdout, stderr = exec_cmd(command)
    if has_issue:
        logger.error(f"Failed to move package, stdout:{stdout},stderr:{stderr}.")
        return COMMAND_FAILED
    logger.info("Successfully move package.")

    # 解压包
    return unpack(file.filename, package_size)


@exter_attack
def upload_databackup(package_size: str, file: UploadFile):
    file_path = os.path.join(HOME_PACKAGE_PATH, file.filename)
    if has_slash(file.filename):
        return COMMAND_FAILED

    destination_path = os.path.join(PACKAGE_PATH, file.filename)

    os.makedirs(os.path.dirname(file_path), exist_ok=True)
    logger.info(f"Start upload file. file_path:{file_path}")
    try:
        with open(file_path, "wb") as file_object:
            for chunk in iter(lambda: file.file.read(MAX_SINGLE_UPLOAD), b""):
                file_object.write(chunk)
    except Exception as e:
        logger.error(f"Failed to upload file, error:{Anonymity.process(str(e))}.")
        return COMMAND_FAILED
    logger.info("Successfully upload file.")

    os.makedirs(os.path.dirname(destination_path), exist_ok=True)

    # 转移包
    logger.info("Start move file.")
    command = ["mv", "-f", file_path, destination_path]
    has_issue, stdout, stderr = exec_cmd(command)
    if has_issue:
        logger.error(f"Failed to move package, stdout:{stdout},stderr:{stderr}.")
        return COMMAND_FAILED
    logger.info("Successfully move package.")

    # 解压包
    return unpack(file.filename, package_size)


def unpack(package_name: str, package_size: str):
    # 判断压缩包类型是否属于合法类型。
    if has_slash(package_name):
        return COMMAND_FAILED
    base_name = get_basename(package_name)
    if not base_name:
        return COMMAND_FAILED
    # 判断package的大小是否正确。若错误，直接退出
    package_path = os.path.join(PACKAGE_PATH, package_name)
    command1 = f'ls -l {package_path}'
    command2 = "awk {'print $5'}"
    has_issue, out = exec_cmds_with_pipe(command1, command2)
    if has_issue:
        logger.error(f"Failed to get package size, error:{out}.")
        return COMMAND_FAILED
    logger.info(f"Successfully get package's size, size is {out}.")
    if out != package_size:
        logger.error(f"Package size wrong ! Real size is {out}, input size is {package_size}")
        return COMMAND_FAILED

    # 先创建解压目标目录
    base_path = os.path.join(PACKAGE_PATH, base_name)
    if os.path.isdir(base_path):
        delete_file(base_path)
    os.mkdir(base_path)

    # 验证package内容合法性
    has_issue = verify_pkg(package_path, base_path)
    if has_issue:
        logger.error(f"Failed to verify package.")
        return COMMAND_FAILED
    logger.info(f"Successfully verify {package_name}.")

    # 解压package
    command = ["tar", "-zxvf", package_path, "-C", base_path]
    has_issue, stdout, stderr = exec_cmd(command)
    if has_issue:
        logger.error(
            f"Failed to unpack package. package path:{package_path}, dir:{base_path}, stdout:{stdout}, stderr:{stderr}")
        return COMMAND_FAILED
    logger.info(f"Successfully unpack package. package path:{package_path}, dir:{base_path}.")

    return COMMAND_SUCCESS


def verify_pkg(pkg_path, destination_path):
    # Simbaos 的包名和其他包的包名不一样
    simbaos_pkg = "SimbaOS" in pkg_path
    try:
        with tarfile.open(pkg_path, 'r:gz') as tar:
            # 1. 验证cms是否正确
            for verify_tool_file in VERIFY_TOOL_LIST:
                if simbaos_pkg:
                    verify_tool_file = verify_tool_file[2:]
                tar.extract(verify_tool_file, destination_path)
            sha256sum_file = os.path.join(destination_path, CHECK_SUM_FILE_NAME)
            crl = os.path.join(destination_path, CRL_FILE_NAME)
            cmd = VERIFY_CHECKSUM_CMD.format(verify_tool=CBB_VERIFY_TOOL, check_sum=sha256sum_file,
                                             cms=CMS_FILE_NAME.format(sha256sum_file), crl=crl)
            ret, res, err = exec_cmd(cmd)
            if ret:
                logger.error(f"Verify image package failed, error: {res}")
                return ret

            # 2. 判断sha256值是否正确
            file_count = 0
            with open(sha256sum_file, "r") as file:
                for line in file:
                    sha256sum, relative_path = line.split()
                    # SimbaOS 包会把sha256sum_sync也打包，同时sum值有问题，因此需要跳过。
                    if relative_path == "./sha256sum_sync":
                        continue
                    file_count += 1
                    if simbaos_pkg:
                        relative_path = relative_path[2:]
                    tf = tar.extractfile(relative_path)
                    real_sum = opened_file_sha256sum(tf)
                    if real_sum != sha256sum:
                        logger.error(f"Sha256 wrong! file is {relative_path}")
                        return COMMAND_FAILED
            # 3. 判断文件数量是否正确
            file_list = [member.name for member in tar.getmembers() if not member.isdir()]
            file_num = len(file_list) - len(VERIFY_TOOL_LIST)
            if file_count != file_num:
                logger.error(f"File num Wrong! Cms file num is {file_count}, file num is {file_num}")
                return COMMAND_FAILED
        return COMMAND_SUCCESS
    except Exception as e:
        logger.error(f"Verify package failed, something error happened:{Anonymity.process(str(e))}")
        return COMMAND_FAILED


def _check_command_injection(input_string):
    danger_params = "?|;&$><`\\!\n\b#"
    for inj in input_string:
        if inj in danger_params:
            logger.error(f"Potential command injection found in string {input_string}")
            return True
    return False


@exter_attack
def delete(package_name: str):
    # 删除包的逻辑是压缩包和目录都删
    base_name = get_basename(package_name)
    if not base_name or has_slash(package_name):
        return COMMAND_FAILED
    package_path = os.path.join(PACKAGE_PATH, package_name)
    unpack_path = os.path.join(PACKAGE_PATH, base_name)

    if delete_file(package_path) == COMMAND_SUCCESS and delete_file(unpack_path) == COMMAND_SUCCESS:
        return COMMAND_SUCCESS

    return COMMAND_FAILED


def get_basename(package_name: str):
    if package_name.endswith(".tgz"):
        base_name = package_name[:-4]
    elif package_name.endswith(".tar.gz"):
        base_name = package_name[:-7]
    else:
        logger.error(f"Failed to unpack package, extension format wrong, only support ['.tar.gz','.tgz']")
        return None
    return base_name


def has_slash(filename):
    if '/' in filename:
        logger.error(f"Bad filename, potential command injection found in string {filename}")
        return True
    return False


def delete_file(path):
    if os.path.exists(path):
        command = ["rm", "-rf", path]
    else:
        logger.warning(f"Path {path} not exist")
        return COMMAND_SUCCESS

    has_issue, stdout, stderr = exec_cmd(command)
    if has_issue:
        logger.error(f'Failed to delete file. Path is: {path}, stdout:{stdout}, stderr:{stderr}.')
        return COMMAND_FAILED

    return COMMAND_SUCCESS


@exter_attack
def unpack_package(hash_value, package_name, package_size):
    package_path = os.path.join(PACKAGE_PATH, package_name)
    if not os.path.isfile(package_path):
        logger.info(f"Package {package_name} not exist")
        return COMMAND_SUCCESS, "not exist", f"Package {package_name} not exist"
    try:
        if hash_value == calculate_file_sha256(package_path):
            logger.info(f"Package {package_name} already exist")
            re = unpack(package_name, package_size)
            return re, "exist", f"Package {package_name} already exist, unpack result is {re}"
        else:
            logger.info(f"Packge {package_name} exist, but not same with input file")
            return COMMAND_SUCCESS, "wrong file", f"Packge {package_name} exist, but not same with input file"
    except Exception as e:
        logger.error(f"Pre check package failed, error is {Anonymity.process(str(e))}")
        return COMMAND_FAILED, "", f"Pre check package failed, error is {Anonymity.process(str(e))}"

