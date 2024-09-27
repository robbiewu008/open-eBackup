import os
import secrets
import string
import pexpect
from random import shuffle
import hashlib

from server.models.bridge import DpserverCertManager
from server.common import consts
#from server.common.dpserver_kmc import Kmcv3Wapper, KmcStatus, encode_base64, decode_base64
from server.common.sync import sync_file


def sha256_of_dir(path, key_files: [str] = None, ):
    if os.path.isdir(path):
        sha256 = hashlib.sha256()
        for root, dirs, files in os.walk(path):
            files.sort()
            for filename in files:
                filepath = os.path.join(root, filename)
                file_relpath = os.path.relpath(filepath, path)
                if key_files is not None and file_relpath not in key_files:
                    continue
                with open(filepath, 'rb') as f:
                    sha256.update(f.read())
        return sha256.hexdigest()
    return None


def generate_key_passwd(length=18):
    letter_chars = string.ascii_lowercase
    capital_chars = string.ascii_uppercase
    digit_chars = string.digits
    punctuation_charts = r"!#$%&()*+,-.:<=>?@[]^_|~"
    quotient, remainder = length // 4, length % 4
    pwd_group = [quotient, quotient, quotient, quotient + remainder]
    password1 = ''.join((secrets.choice(letter_chars) for _ in range(pwd_group[0])))
    password2 = ''.join((secrets.choice(capital_chars) for _ in range(pwd_group[1])))
    password3 = ''.join((secrets.choice(digit_chars) for _ in range(pwd_group[2])))
    password4 = ''.join((secrets.choice(punctuation_charts) for _ in range(pwd_group[3])))
    password = password1 + password2 + password3 + password4
    pass_list = list(password)
    shuffle(pass_list)
    return ''.join(pass_list)


class CertManagerDataBackup(DpserverCertManager):
    def __init__(self):
        super().__init__()
        self.passwd = ""

    def get_private_cert_passwd(self):
        """

        """
        self.passwd = generate_key_passwd()

    def prepare_cert(self):
        """
        Generate self-signed cert, include key file and cert file.
        """
        if os.path.isfile(consts.SSL_KEYFILE_PATH_DPSERVER) and \
                os.path.isfile(consts.SSL_CERTFILE_PATH_DPSERVER) and \
                os.path.isfile(consts.SSL_CA_CERTFLE_PATH_DPSERVER):
            return
        os.makedirs(consts.DPSERVER_CERT_BASE, exist_ok=True)
        p1 = pexpect.spawn(
            f'openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 -aes256 '
            f'-out {consts.SSL_KEYFILE_PATH_DPSERVER}', encoding='utf-8',
            env={"LD_LIBRARY_PATH": ''})
        p1.expect(f"Enter PEM pass phrase:")
        p1.sendline(self.passwd)
        p1.expect(f"Verifying - Enter PEM pass phrase:")
        p1.sendline(self.passwd)
        p1.expect(pexpect.EOF)
        p1.wait()

        fields = [
            (f"Enter pass phrase for {consts.SSL_KEYFILE_PATH_DPSERVER}:", self.passwd),
            ("You are .*:", ""),
            ("State or .*:", ""),
            ("Locality .*:", ""),
            ("Organization Name .*:", ""),
            ("Organizational Unit .*:", ""),
            ("Common Name .*:", ""),
            ("Email Address .*:", "")
        ]
        p2 = pexpect.spawn(
            f'openssl req -new -x509 -key {consts.SSL_KEYFILE_PATH_DPSERVER} '
            f'-out {consts.SSL_CERTFILE_PATH_DPSERVER} -days {consts.CERT_VALIDITY_PERIOD}', encoding='utf-8',
            env={"LD_LIBRARY_PATH": ''})
        for prompt, response in fields:
            p2.expect(prompt)
            p2.sendline(response)
        p2.wait()

        sync_file(consts.SSL_CERTFILE_PATH_DPSERVER, consts.SSL_CA_CERTFLE_PATH_DPSERVER)

    def get_cert_sha256(self, cert_dir: str = consts.DPSERVER_CERT_BASE):
        dir_sha256 = sha256_of_dir(cert_dir)
        return dir_sha256

    def renew_cert(self):
        pass
