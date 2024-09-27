# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#!/usr/bin/env python
# _*_ coding:utf-8 _*_
# Obtaining the ip address of the PM service plane

import os
import re
import json
import time
import ipaddress
import logging
import ssl
import subprocess
import shlex
import operator
from urllib import request
from kmc_util import Kmc, SecurityConstants, read_file_by_utf_8

TIMEOUT = 60
DEFAULT_TRY_NUM = 3
GLOBAL_KMC_INIT_FLAG = False
NAMESPACE = "dpa"
INFRASTRUCTURE_NAME = "infrastructure-[0-9]"
INFRASTRUCTURE_INFO = "https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/internal/pod/info?appName=infrastructure"
AGENT_ROOT_PATH ="/opt/DataBackup/ProtectClient/ProtectClient-E"
# backup_net_plane
NET_WORK_CONF = '/opt/network-conf/backup_net_plane'

# ssl config
INTERNAL_CERT_DIR = "/opt/logpath/infrastructure/cert/internal"
INTERNAL_CA_FILE = f"{INTERNAL_CERT_DIR}/ca/ca.crt.pem"
INTERNAL_CERT_FILE = f"{INTERNAL_CERT_DIR}/internal.crt.pem"
INTERNAL_KEY_FILE = f"{INTERNAL_CERT_DIR}/internal.pem"

# password
KMC_MASTER_KS_PATH = "/opt/logpath/protectmanager/kmc/master.ks"
KMC_BACKUP_KS_PATH = "/kmc_conf/..data/backup.ks"
INTERNAL_KEYFILE_PWD_FILE = f"{INTERNAL_CERT_DIR}/internal_cert"

logging.basicConfig(level=logging.INFO,
                    filename=f"{AGENT_ROOT_PATH}/log/getpmip.log",
                    format='[%(asctime)s-%(levelname)s] %(message)s [%(filename)s:%(lineno)d][%(process)d]',
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

def get_pm_ip():
    """
    Get pm ip from the file /opt/network-conf/backup_net_plane
    Exapmle:[{"nodeId":"node-0","logic_ip_list":[{"ip":"192.168.194.1","mask":"255.255.0.0"}],
            "ips_route_table":[{"ip":"192.168.194.1","routes":[
                {"type":"2","destination":"0.0.0.0","mask":"0.0.0.0","gateway":"192.168.1.0"},
                {"type":"1","destination":"1.1.1.1","mask":"255.255.255.255","gateway":"192.168.1.2"},
                {"type":"0","destination":"1.2.0.0","mask":"255.255.0.0","gateway":"192.168.1.3"}]}]},
        {"nodeId":"node-1","logic_ip_list":[{"ip":"192.168.194.2","mask":"255.255.0.0"}],
            "ips_route_table":[{"ip":"192.168.194.2","routes":[]}]}]
    """
    PM_COUNT=0
    PM_IP = ""
    Local_IP = ""
    pm_ip_list = list()
    local_ip_list = list()
    with open(f'{NET_WORK_CONF}', 'r') as f:
        netplane_info_str = f.read()
        if not netplane_info_str:
            return PM_IP, Local_IP, PM_COUNT
        netplane_info = json.loads(netplane_info_str)
        for netplane in netplane_info:
            PM_COUNT += 1
            local_host_name = os.environ.get('NODE_NAME')
            node_id = netplane.get('nodeId')
            logic_ip_list = netplane.get('logic_ip_list')
            pm_ip_list.extend(logic_ip_list)
            if local_host_name != node_id:
                continue
            local_ip_list.extend(logic_ip_list)
        for ip_content in pm_ip_list:
            ip = ip_content.get("ip")
            PM_IP = PM_IP + ',' + ip
        for ip_content in local_ip_list:
            ip = ip_content.get("ip")
            Local_IP = Local_IP + ',' + ip
    PM_IP = PM_IP.strip(',')
    Local_IP = Local_IP.strip(',')
    return PM_IP, Local_IP, PM_COUNT

def get_infra_info():
    url = INFRASTRUCTURE_INFO
    err_code, data = get_data_from_api(url)
    if err_code or not data:
        logging.warning(f"Get data for '{url}' failed")
        return ""

    #logging.warning(f"Get data result: {data}")
    node_name=""
    for msg in data:
        # namespace
        pod_namespace=msg.get("namespace", "")
        if pod_namespace != NAMESPACE:
            continue

        # podName
        pod_name=msg.get("podName", "")
        if not re.match(INFRASTRUCTURE_NAME, msg.get("podName", "")):
            continue

        # netPlaneInfo
        node_name=msg.get("nodeName", "")
        if node_name:
            break
    return node_name

def main():
    ip_address, local_ip, count = get_pm_ip()
    print(ip_address)
    print(local_ip)
    print(count)
    if ip_address:
        logging.info(f"The ip address of the pm is obtained successfully.[{ip_address}].")
    else:
        logging.warning("Get data ip failed, the data ip is empty.")
    node_name = get_infra_info()
    print(node_name)
    if node_name:
        logging.info(f"Get infra info node successfully [{node_name}].")
    return ip_address, node_name, count, local_ip

if __name__ == "__main__":
    main()