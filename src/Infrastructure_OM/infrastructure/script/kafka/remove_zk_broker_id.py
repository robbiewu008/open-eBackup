#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import os
import sys

from kazoo.client import KazooClient

from kazoo.exceptions import NoNodeError

from common.logger.logger import get_logger
from common.retry import retry_func

node_name = os.getenv('NODE_NAME')
pod_ip = os.getenv('POD_IP')
log_file = os.path.join(os.sep, 'opt', 'OceanProtect', 'logs', node_name,
                        'infrastructure', 'kafka', 'install_kafka.log')
logger = get_logger(log_file)


@retry_func
def main():
    if len(sys.argv) > 1:
        broker = sys.argv[1]
    else:
        logger.error("No broker ids provided.")
        exit(1)
    zk = KazooClient(hosts=f'{pod_ip}:2181')
    logger.info('Start initiate connection to ZK.')
    try:
        zk.start()
        logger.info(f'Start delete /brokers/ids/{broker} in zookeeper.')
        zk.delete(f'/brokers/ids/{broker}')
        logger.info(f'Delete /brokers/ids/{broker} in zookeeper successfully.')
    except NoNodeError:
        logger.warning(f'/brokers/ids/{broker} not exist, no need to delete.')
    except Exception as e:
        msg = f'Delete /brokers/ids/{broker} in zookeeper failed. {e}'
        logger.exception(msg)
        raise Exception(msg)
    finally:
        zk.stop()


if __name__ == '__main__':
    main()