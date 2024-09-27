# coding: utf-8
import ctypes
import json
import os
import shlex
import ssl
import subprocess
import sys
from urllib import request

import pexpect

from common.logger.logger import get_logger
from common.utils import clear, check_path_validity
from kmc.common import kmc_decrypt, initialize_kmc

node_name = os.getenv('NODE_NAME')
log_file = os.path.join(os.sep, 'opt', 'OceanProtect', 'logs', node_name,
                        'infrastructure', 'gaussdb', 'install_gaussdb.log')
logger = get_logger(log_file)

SECRET_URL = "https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/secret/info" \
             "?nameSpace=dpa&secretName=common-secret"

CA_FILE = "/opt/OceanProtect/infrastructure/cert/internal/ca/ca.crt.pem"
CERT_FILE = "/opt/OceanProtect/infrastructure/cert/internal/internal.crt.pem"
KEY_FILE = "/opt/OceanProtect/infrastructure/cert/internal/internal.pem"
INFRA_CERT = "/opt/OceanProtect/infrastructure/cert/internal/internal_cert"

GAUSSDB_PORT = 6432
GAUSSDB_URL = "gaussdb.dpa.svc.cluster.local"
INFRASTRUCTURE_SECRET_URL = "https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/secret/info" \
                            "?nameSpace=dpa&secretName=common-secret"
TABLE_MAP = {"dme_unified": ["task", "task_index_seq"], "protect_manager": ["t_job"]}


def release_str_memory(sensitive_str):
    """
    释放敏感字符串内存，已确认，调用方法出的传参也可以释放内存
    :param sensitive_str: 敏感信息对象，
    :return: 无
    """
    buf_size = len(sensitive_str) + 1
    offset = sys.getsizeof(sensitive_str) - buf_size
    ctypes.memset(id(sensitive_str) + offset, 0, buf_size)


def get_db_pwd_from_api(db_pwd_field):
    """
    通过infrastructure接口获取指定数据库密码字段的值
    :param db_pwd_field: 数据密码字段
    :return: 请求返会内容
    """
    context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
    context.check_hostname = False
    context.load_cert_chain(
        certfile=CERT_FILE,
        keyfile=KEY_FILE,
        password=kmc_decrypt(INFRA_CERT, logger)
    )
    context.load_verify_locations(cafile=CA_FILE)
    context.verify_mode = ssl.CERT_REQUIRED
    headers = {"Content-Type": "application/json"}
    req = request.Request(SECRET_URL, method="GET", headers=headers)
    response = request.urlopen(req, context=context)

    try:
        data = json.loads(response.read().decode('utf-8')).get("data")
    except Exception:
        logger.error(f'get data from url response failed.')
        raise

    for tmp_dict in data:
        if db_pwd_field in tmp_dict:
            return tmp_dict.get(db_pwd_field)

    logger.error(f'get db_pwd_field:{db_pwd_field} failed.')
    return ""


def exec_cmd(command):
    """
    执行shell命令
    :param command:
    :return:
    """
    process = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding="utf-8")
    if process.poll() == 0:
        return True
    out = process.stdout.read()
    err = process.stderr.read()
    if "total time" in out or "total time" in err:
        return True
    else:
        logger.error(f"stdout:{out}, stderr:{err}.")
        return False


def main(master_ks, backup_ks, manage_type, db_name, usr_name, pwd_field, file_path, manage_ip):
    """
    主函数
    :param master_ks: 密钥文件
    :param backup_ks: 密钥文件
    :param manage_type: 数据管理类型。导入、导出
    :param db_name: 数据库名称
    :param usr_name: 用户名
    :param pwd_field: 密码字段
    :param file_path: 导入、导出文件路径
    :param manage_ip: 管理ip
    """
    logger.info('Start manage db:%s data.', db_name)
    initialize_kmc(master_ks, backup_ks, logger)
    # 校验file_path是合法的路径
    file_path_pre = file_path.split(".")[0]
    if not check_path_validity(file_path_pre):
        logger.error("File path:%s is invalid", file_path)
        os._exit(1)
    pwd = get_db_pwd_from_api(pwd_field)
    logger.info("Get db pwd from api success")
    try:
        exec_sql_cmd(db_name, file_path, manage_ip, manage_type, pwd, usr_name)
    finally:
        clear(pwd)


def exec_sql_cmd(db_name, file_path, manage_ip, manage_type, pwd, usr_name):
    if manage_type == 'import':
        import_cmd = f'/usr/local/gaussdb/app/bin/gsql {db_name} -p {GAUSSDB_PORT}  -U {usr_name} ' \
                     f'-f {file_path}'
        execute_cmd_with_expect(import_cmd, pwd)
    elif manage_type == 'export':
        filter_db = ""
        for table in TABLE_MAP.get(db_name, ""):
            filter_db += f"--exclude-table-data={table} "
        export_cmd = f'/usr/local/gaussdb/app/bin/gs_dump {db_name} -p {GAUSSDB_PORT} -h {GAUSSDB_URL} ' \
                     f'-U {usr_name} -W \'{pwd}\' -f {file_path} {filter_db}'
        exec_cmd(export_cmd)
    else:
        logger.error(f'manage_type:{manage_type} param error.')
        os._exit(1)


def execute_cmd_with_expect(cmd, pwd):
    child_proc = pexpect.spawnu(cmd, timeout=100)
    try:
        if child_proc.expect(r"Password") != 0:
            raise Exception('The password is invalid.')
        child_proc.buffer = ""
        child_proc.sendline(pwd)
        index = child_proc.expect(pexpect.EOF, timeout=10800)
        if index != 0:
            raise Exception('Manage data execute cmd failed.')
    finally:
        if child_proc:
            child_proc.close()


if __name__ == "__main__":
    if len(sys.argv) != 9:
        logger.error(f'count of params not right.')
        os._exit(1)
    main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6], sys.argv[7], sys.argv[8])