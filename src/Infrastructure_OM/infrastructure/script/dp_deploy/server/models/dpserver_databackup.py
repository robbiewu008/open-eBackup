import logging
from fastapi import HTTPException, Header
import paramiko
import os
import subprocess
import requests
import json
from typing import Tuple

from server.models.bridge import DpServerBase
from server.models.cert_manager_databackup import CertManagerDataBackup
from server.common import consts
from server.common.get_auth_token import get_auth_token
from server.api.rest.routers import databackup_register

class DpServerDataBackup(DpServerBase):

    def __init__(self, address):
        super().__init__(address)

    def prepare_env(self, context):
        open_port_path = os.path.join(consts.SCRIPTS_PATH, "add_firewall_ports.sh")
        cmd = subprocess.run(
            f'/bin/sh {open_port_path}',
            env={'PYTHONHOME': '/usr'},
            shell=True,
            capture_output=True,
        )
        if cmd.returncode != 0:
            logging.warning("Open firewall port failed, "
                            f"out: {cmd.stdout}, error:{cmd.stderr}")

    def credentials(self, ssh_user: str = Header(default=None), ssh_passwd: str = Header(default=None),
                    ip: str = Header(default=None), op_admin_user: str = Header(default=None),
                    op_admin_pwd: str = Header(default=None)):
        if ip and op_admin_pwd and op_admin_user:
            if get_auth_token(ip, op_admin_user, op_admin_pwd):
                logging.info(f"Successfully get the auth token")
                return
            logging.error(f"Fail to get auth token: ip is {ip}, op_admin_user is {op_admin_user}, "\
                          f"op_admin_pwd is {op_admin_pwd}")
            raise HTTPException(status_code=401)
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())

        try:
            client.connect(self.address, consts.SSH_DEFAULT_PORT, ssh_user, ssh_passwd)
        except Exception:
            logging.error(f"Credential failed")
            raise HTTPException(status_code=401)

    def generate_route(self, api):
        databackup_register(api)
