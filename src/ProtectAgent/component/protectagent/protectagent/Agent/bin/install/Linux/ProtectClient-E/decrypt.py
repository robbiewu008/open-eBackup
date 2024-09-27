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

AGENT_ROOT_PATH ="/opt/DataBackup/ProtectClient/ProtectClient-E"
OPENAPI_CNF_FILE = "/opt/logpath/protectmanager/cert/internal/ProtectAgent/client.cnf"
INFRA_REST_URL = "https://pm-system-base:30081/v1/kms/decrypt"

DEFAULT_TRY_NUM = 3
GLOBAL_KMC_INIT_FLAG = False

logging.basicConfig(level=logging.INFO,
                    filename=f"{AGENT_ROOT_PATH}/log/decrypt.log",
                    format='[%(asctime)s-%(levelname)s] %(message)s [%(filename)s-%(lineno)d-%(process)d]',
                    datefmt='%Y-%m-%d %H:%M:%S')

def decrypt(url):
    context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
    context.check_hostname = False
    context.load_cert_chain(
        certfile=SecurityConstants.INTERNAL_CERT_FILE,
        keyfile=SecurityConstants.INTERNAL_KEY_FILE,
        password=Kmc().decrypt(read_file_by_utf_8(SecurityConstants.INTERNAL_KEYFILE_PWD_FILE))
    )
    context.load_verify_locations(cafile=SecurityConstants.INTERNAL_CA_FILE)
    context.verify_mode = ssl.CERT_REQUIRED

    with open(OPENAPI_CNF_FILE, 'r') as file_obj:
        content = file_obj.read()
    if not content:
        logging.error(f"Failed to read the file {OPENAPI_CNF_FILE}.")
        sys.exit(1)

    req_data = {
        "ciphertext": content
    }
    headers = {'Content-Type': 'application/json'}
    method="POST"
    req = request.Request(url, data=json.dumps(req_data).encode(), method=method, headers=headers)
    for i in range(DEFAULT_TRY_NUM):
        try:
            res = request.urlopen(req, context=context)
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

        data = res_json.get('plaintext', "")
        if not data:
            logging.warning(f"Get data for '{url}' is empty")
            time.sleep(2)
            return 1, ""

        return 0, data

    return 1, ""

def main():
    private_key = decrypt(INFRA_REST_URL)[1]
    if len(private_key) == 0:
        logging.warning(f"Get password is empty")
        sys.exit(1)
        
    cmd1 = f"echo {private_key}"
    p_open1 = subprocess.Popen(shlex.split(cmd1), stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                               shell=False, encoding="utf-8")

    cmd2 = f"{AGENT_ROOT_PATH}/bin/agentcli enckey cipherFile"
    p_open2 = subprocess.Popen(shlex.split(cmd2), stdin=p_open1.stdout, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, shell=False, encoding="utf-8")
    out, err = p_open2.communicate()
    returncode = p_open2.returncode
    if err:
        logging.error("Enckey fail. Error Message: {err}")
        raise Exception(f"Enckey fail. Error Message: {err}.")
    if returncode != 0:
        logging.error("Enckey fail. returncode: {returncode}")
        raise Exception(f"Enckey fail. returncode: {returncode}.")

if __name__ == "__main__":
    main()
    logging.info("Decrypt succ")
    sys.exit(0)
