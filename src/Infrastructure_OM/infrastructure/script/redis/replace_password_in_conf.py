import base64
import os
import re
import shutil
import sys
import tempfile

from common.logger.logger import get_logger
from kmc.common import release_str_memory, kmc_decrypt, initialize_kmc
from redis_consts import REDIS_PWD_SECRET_PATH, REDIS_CONF_IN_NAS

node_name = os.getenv('NODE_NAME')
log_file = os.path.join(os.sep, 'opt', 'OceanProtect', 'logs', node_name,
                        'infrastructure', 'redis', 'install_redis.log')
logger = get_logger(log_file)


def decode_base64(data):
    """
    将data进行base64解码
    :param data:
    :return:
    """
    decoded_bytes = base64.b64decode(data.encode("utf-8"))
    return str(decoded_bytes, "utf-8")


def replace_in_file(file_path: str, pattern: str, new_text: str):
    with tempfile.NamedTemporaryFile(mode='w', delete=False) as tmp_file:
        with open(file_path) as origin_file:
            for line in origin_file:
                tmp_file.write(re.sub(pattern, new_text, line))
    shutil.copystat(file_path, tmp_file.name)
    shutil.move(tmp_file.name, file_path)


def replace_redis_password(new_password: str):
    """替换redis.conf中的密码"""
    logger.info(f'Start replace password of redis.conf.')

    # new_password 应该被双引号包括(配置文件格式要求)
    replace_in_file(REDIS_CONF_IN_NAS, r'^[#\s]*requirepass.*', f'requirepass "{new_password}"')
    replace_in_file(REDIS_CONF_IN_NAS, r'^[#\s]*masterauth.*', f'masterauth "{new_password}"')


def main(master_ks: str, backup_ks: str):
    logger.info(f'Start update redis password.')
    initialize_kmc(master_ks, backup_ks, logger)
    new_password = kmc_decrypt(REDIS_PWD_SECRET_PATH, logger)
    replace_redis_password(new_password)
    release_str_memory(new_password)
    logger.info('Redis password updated successfully.')


if __name__ == '__main__':
    try:
        if len(sys.argv) != 3:
            msg = f"Wrong number of parameters. It should be 3, " \
                  f"but given {len(sys.argv)}"
            logger.error(msg)
            raise Exception(msg)
        main(sys.argv[0], sys.argv[1])
    except Exception as e:
        logger.exception(f'Update redis password process failed! {e}')
        os._exit(1)
