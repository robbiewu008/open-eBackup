import os
import shutil
from server.models.bridge import DpserverCertManager
from server.common.logger.logger import logger
from server.common import consts
from server.common.local_kmc import decrypt_private_key_passwd


class CertManagerE6000(DpserverCertManager):
    def __init__(self, ):
        super().__init__()

    def get_private_cert_passwd(self):
        key_passwd_path = consts.SSL_KEYFILE_PASSWORD_UNDECRYPT_DPSERVER
        if not os.path.isfile(key_passwd_path):
            shutil.copyfile(consts.SSL_KEYFILE_PASSWORD_UNDECRYPT_PACIFIC, key_passwd_path)

        kmc_lib_path = consts.KMC_LIB_PATH_DPSERVER
        if not os.path.isfile(kmc_lib_path):
            shutil.copyfile(consts.KMC_LIB_PATH_PACIFIC, kmc_lib_path)

        key_pass = decrypt_private_key_passwd(key_passwd_path)
        self.passwd = key_pass

    def prepare_cert(self):
        pass

    def renew_cert(self):
        pass

    def get_cert_sha256(self, cert_dir: str = None):
        pass
