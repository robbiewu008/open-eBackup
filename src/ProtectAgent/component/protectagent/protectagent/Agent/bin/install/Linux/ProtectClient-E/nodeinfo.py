import os
import re
import json
import time
import ipaddress
import logging
import ssl
import subprocess
import shlex
from urllib import request
from kmc_util import Kmc, SecurityConstants, read_file_by_utf_8

TIMEOUT = 60
DEFAULT_TRY_NUM = 3

INFRASTRUCTURE_DOMAIN_NAME="infrastructure.dpa.svc.cluster.local"
AGENT_ROOT_PATH ="/opt/DataBackup/ProtectClient/ProtectClient-E"
NODE_INFO_URL = f"https://{INFRASTRUCTURE_DOMAIN_NAME}:8088/v1/infra/collect/node/info"

logging.basicConfig(level=logging.INFO,
                    filename=f"{AGENT_ROOT_PATH}/log/nodeinfo.log",
                    format='[%(asctime)s-%(levelname)s] %(message)s [%(filename)s-%(lineno)d-%(process)d]',
                    datefmt='%Y-%m-%d %H:%M:%S')

def get_data_from_api(url):
    context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
    context.check_hostname = False
    context.load_cert_chain(
        certfile=SecurityConstants.INTERNAL_CERT_FILE,
        keyfile=SecurityConstants.INTERNAL_KEY_FILE,
        password=Kmc().decrypt(read_file_by_utf_8(SecurityConstants.INTERNAL_KEYFILE_PWD_FILE))
    )
    context.load_verify_locations(cafile=SecurityConstants.INTERNAL_CA_FILE)
    context.verify_mode = ssl.CERT_REQUIRED

    for i in range(DEFAULT_TRY_NUM):
        try:
            res = request.urlopen(url, context=context)
        except Exception as ex:
            logging.warning(f"Connect '{url}' failed, error: {ex}")
            time.sleep(2)
            continue

        res_json = {}
        try:
            res_data = res.read()
            if res_data:
                res_json = json.loads(res_data.decode('utf-8'))
        except Exception as ex:
            logging.warning(f"Parse result for url failed, error: {ex}")
            time.sleep(2)
            continue

        data = res_json.get('data', [])
        if not data:
            logging.warning(f"Get data for '{url}' is empty")
            time.sleep(2)
            return 1, []

        return 0, data

    return 1, []

def get_node_info(url):
    """
    从infrastructure api接口中解析数据面ip地址
    :param url: REST_URL
    :return: str
    """
    err_code, data = get_data_from_api(url)
    if err_code or not data:
        logging.warning(f"Get data for '{url}' failed")
        return ""

    return data

def main():
    nodeInfo = get_node_info(NODE_INFO_URL)
    print(json.dumps(nodeInfo, sort_keys=True, indent=4))
    if nodeInfo:
        logging.info(f"The ip address of the pm is obtained successfully.[{nodeInfo}].")
    return nodeInfo

if __name__ == "__main__":
    main()
