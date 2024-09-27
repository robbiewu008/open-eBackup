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
import ctypes
import os
import logging
from enum import Enum
from threading import Lock

__all__ = ["Kmc"]
GLOBAL_KMC_INIT_FLAG = False

AGENT_ROOT_PATH ="/opt/DataBackup/ProtectClient/ProtectClient-E"

logging.basicConfig(level=logging.INFO,
                    filename=f"{AGENT_ROOT_PATH}/log/kmc_util.log",
                    format='[%(asctime)s-%(levelname)s] %(message)s [%(filename)s-%(lineno)d-%(process)d]',
                    datefmt='%Y-%m-%d %H:%M:%S')

def encode_text(text):
    if isinstance(text, bytes):
        return text
    else:
        return text.encode("UTF-8")

def decode_text(text):
    if isinstance(text, bytes):
        return text.decode("UTF-8")
    else:
        return text

def read_file_by_utf_8(file_path):
    if not file_path or not os.path.exists(file_path):
        raise ValueError("file path error")
    with open(file_path, "r", encoding="utf-8") as f:
        text = f.read()
    return text

class KmcStatus(int, Enum):
    KMC_SUCCESS = 0
    KMC_FAIL = 1
    KMC_ENCTXT_INVAILD = 2  # 无效密文错误码

class SecurityConstants:
    INTERNAL_CERT_DIR = "/opt/logpath/infrastructure/cert/internal"
    INTERNAL_CA_FILE = f"{INTERNAL_CERT_DIR}/ca/ca.crt.pem"
    INTERNAL_CERT_FILE = f"{INTERNAL_CERT_DIR}/internal.crt.pem"
    INTERNAL_KEY_FILE = f"{INTERNAL_CERT_DIR}/internal.pem"
    INTERNAL_KEYFILE_PWD_FILE = f"{INTERNAL_CERT_DIR}/internal_cert"

    # 依赖的KMC动态库
    LIBKMCV3_SO_PATH = "/usr/lib64/libkmcv3.so"
    DEFAULT_DOMAIN_ID = 0
    MODULE_NAME = "infrastructure_om"
    KMC_MASTER_KS_PATH = "/opt/logpath/protectmanager/kmc/master.ks"
    KMC_BACKUP_KS_PATH = "/kmc_conf/..data/backup.ks"

def build_kmc_handler():
    try:
        handler = ctypes.cdll.LoadLibrary(SecurityConstants.LIBKMCV3_SO_PATH)
        global GLOBAL_KMC_INIT_FLAG
        if GLOBAL_KMC_INIT_FLAG:
            logging.warning("KMC has been initialized, finalize first.")
            de_init_ret = handler.DeInitKmc()
            if de_init_ret != KmcStatus.KMC_SUCCESS:
                logging.error("KMC finalize failed.")
                raise Exception("KMC finalize failed.")
            GLOBAL_KMC_INIT_FLAG = False
            logging.info("KMC finalize success.")
        if not os.path.exists(SecurityConstants.KMC_MASTER_KS_PATH)\
                or not os.path.exists(SecurityConstants.KMC_BACKUP_KS_PATH):
            logging.error("KMC keystore file does not exist.")
            raise Exception("KMC keystore file does not exist.")
        init_ret = handler.InitKMCV3c(encode_text(SecurityConstants.KMC_MASTER_KS_PATH),
                                      encode_text(SecurityConstants.KMC_BACKUP_KS_PATH),
                                      encode_text(SecurityConstants.MODULE_NAME))
        if init_ret != KmcStatus.KMC_SUCCESS:
            logging.error(f"KMC initialize failed. Error Code: {init_ret}.")
            raise Exception("KMC initialize failed.")
        GLOBAL_KMC_INIT_FLAG = True
        logging.info("KMC initialize success.")
        return handler
    except Exception as ex:
        logging.exception("KMC initialize failed.")
        raise ex
    finally:
        pass

class Kmc:
    _instance_lock = Lock()
    _instance = None
    _handler = None

    def __new__(cls, *args, **kwargs):
        if cls._instance:
            cls._instance
        with cls._instance_lock:
            if cls._instance is None:
                cls._instance = object.__new__(cls)
                cls._handler = build_kmc_handler()
        return cls._instance

    def encrypt(self, plain_text: str, domain_id=SecurityConstants.DEFAULT_DOMAIN_ID) -> str:
        """
        加密明文字符串
        :param plain_text: str 需要加密的明文字符串
        :param domain_id: int 加密使用的域ID
        :return: str 密文字符串
        """
        if not plain_text:
            logging.warning("Cipher text is None.")
            return plain_text
        cipher = ctypes.c_char_p()
        try:
            p_cipher = ctypes.pointer(cipher)
            result = self._handler.EncryptV3c(domain_id, encode_text(plain_text), p_cipher)
            if result != KmcStatus.KMC_SUCCESS:
                logging.error(f"KMC encrypt failed. Error Code: {result}.")
                raise Exception("Kmc decrypt failed")
            cipher_text = decode_text(cipher.value)
        finally:
            # 失败底层会释放指针
            self._handler.KmcFree(cipher)
        return cipher_text

    def decrypt(self, cipher_text: str, domain_id=SecurityConstants.DEFAULT_DOMAIN_ID) -> str:
        """
        解密密文字符串
        :param cipher_text: str 密文字符串
        :param domain_id: int 解密使用的域ID
        :return: str 明文字符串
        """
        if not cipher_text:
            logging.warning("cipher text is None.")
            return cipher_text
        plain = ctypes.c_char_p()
        try:
            p_plain = ctypes.pointer(plain)
            result = self._handler.DecryptV3c(domain_id, p_plain, encode_text(cipher_text))
            if result != KmcStatus.KMC_SUCCESS:
                logging.error(f"KMC decrypt failed. Error Code: {result}.")
                raise Exception("Kmc decrypt failed")
            plain_text = decode_text(plain.value)
        finally:
            # 失败底层会释放指针
            self._handler.KmcFree(plain)
        return str(plain_text).strip() if plain_text else plain_text
