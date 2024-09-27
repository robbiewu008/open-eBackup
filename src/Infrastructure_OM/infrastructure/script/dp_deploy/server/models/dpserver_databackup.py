import logging
from fastapi import HTTPException, Header
import paramiko

from server.models.bridge import DpServerBase
from server.models.cert_manager_databackup import CertManagerDataBackup
from server.common import consts
from server.api.rest.routers import databackup_register

class DpServerDataBackup(DpServerBase):

    def __init__(self, address):
        super().__init__(address)

    def prepare_env(self, context):
        return None

    def credentials(self, ssh_user: str = Header(default=None), ssh_passwd: str = Header(default=None)):
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())

        try:
            client.connect(self.address, consts.SSH_DEFAULT_PORT, ssh_user, ssh_passwd)
        except Exception:
            logging.error(f"Credential failed")
            raise HTTPException(status_code=401)

    def generate_route(self, api):
        databackup_register(api)
