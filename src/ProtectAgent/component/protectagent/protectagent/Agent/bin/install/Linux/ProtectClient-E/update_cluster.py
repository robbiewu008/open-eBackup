#!/usr/bin/env python
# coding: utf-8
import os
import json
import time
import ipaddress
import logging
import ssl
import subprocess
import shlex
import sys
from urllib import request
from kmc_util import Kmc, SecurityConstants, read_file_by_utf_8
from getpmip import get_pm_ip

AGENT_ROOT_PATH ="/opt/DataBackup/ProtectClient/ProtectClient-E"
OPENAPI_CNF_FILE = "/opt/logpath/protectmanager/cert/internal/OpenAPI/OpenAPI.cnf"
INFRA_REST_URL = "https://pm-system-base.dpa.svc.cluster.local:30081/v1/internal/agent/cluster-ip"

DEFAULT_TRY_NUM = 3
GLOBAL_KMC_INIT_FLAG = False

###### 防勒索
DEPLOY_TYPE_HYPERDETECT = "d4"
DEPLOY_TYPE_CYBER_ENGINE = "d5"

def update_cluster_ip(url):
    context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
    context.check_hostname = False
    context.load_cert_chain(
        certfile=SecurityConstants.INTERNAL_CERT_FILE,
        keyfile=SecurityConstants.INTERNAL_KEY_FILE,
        password=Kmc().decrypt(read_file_by_utf_8(SecurityConstants.INTERNAL_KEYFILE_PWD_FILE))
    )
    context.load_verify_locations(cafile=SecurityConstants.INTERNAL_CA_FILE)
    context.verify_mode = ssl.CERT_REQUIRED

    deploy_type = os.getenv("DEPLOY_TYPE")
    if deploy_type == DEPLOY_TYPE_HYPERDETECT or deploy_type == DEPLOY_TYPE_CYBER_ENGINE:
        logging.info("The environment(hyperdetect) does not support updating the IP address of the PM.")
        return True

    ip_list, _ = get_pm_ip()
    if not ip_list:
        logging.error("Failed to query the PM service IP address.")
        return False
    ip_list = ip_list.split(",")
    logging.info(f"ip_list: {ip_list}")

    req_content = dict()
    req_content["managerServerList"] = ip_list
    headers = {'Content-Type': 'application/json'}
    method="PUT"
    req = request.Request(url, data=json.dumps(req_content).encode(), method=method, headers=headers)
    req_count = 0
    while req_count < DEFAULT_TRY_NUM:
        req_count += 1
        try:
            res = request.urlopen(req, context=context)
        except Exception as ex:
            logging.warning(f"Connect '{url}' failed, error: {ex}")
            time.sleep(2)
            continue
        return True
    return False

def main():
    result = update_cluster_ip(INFRA_REST_URL)
    if not result:
        logging.error("Update cluster-ip failed.")
        sys.exit(1)
    logging.info("Update succ.")

if __name__ == "__main__":
    main()
