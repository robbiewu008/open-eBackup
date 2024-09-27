#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import os
import sys

from common.logger.logger import get_logger
from kafka_consts import KAFKA_PWD_PATH, KAFKA_USER_PATH, KAFKA_JAAS_PATH
from kmc.common import kmc_decrypt, initialize_kmc

node_name = os.getenv('NODE_NAME')
log_file = os.path.join(os.sep, 'opt', 'OceanProtect', 'logs', node_name,
                        'infrastructure', 'kafka', 'install_kafka.log')
logger = get_logger(log_file)


def get_kafka_user_name_from_secrets():
    with open(KAFKA_USER_PATH, 'r') as file:
        encoded_user_name = file.read()
        return encoded_user_name


def main(master_ks: str, backup_ks: str):
    logger.info(f'Start update kafka jaas conf.')
    initialize_kmc(master_ks, backup_ks, logger)
    kafka_username = get_kafka_user_name_from_secrets()
    kafka_password = kmc_decrypt(KAFKA_PWD_PATH, logger)

    args = {'kafka_username': kafka_username,
            'kafka_password': kafka_password}
    with open(KAFKA_JAAS_PATH, 'w') as f:
        f.write('''KafkaServer {{
        org.apache.kafka.common.security.plain.PlainLoginModule required
        username="{kafka_username}"
        password="{kafka_password}"
        user_{kafka_username}="{kafka_password}";
        }};'''.format(**args))


if __name__ == '__main__':
    try:
        if len(sys.argv) != 3:
            msg = f"Wrong number of parameters. It should be 3, " \
                  f"but given {len(sys.argv)}"
            logger.error(msg)
            raise Exception(msg)
        main(sys.argv[0], sys.argv[1])
    except Exception as e:
        logger.exception(f'Update kafka jaas conf failed! {e}')
        os._exit(1)
