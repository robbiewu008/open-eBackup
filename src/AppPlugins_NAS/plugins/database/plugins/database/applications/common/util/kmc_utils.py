#
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
#

import ctypes
import os
from enum import Enum
from threading import Lock

from common.constants.constants import SecurityConstants
from common.env_common import get_install_head_path

GLOBAL_KMC_INIT_FLAG = False
# libkmcv3.so LIB路径
LIB_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/lib/libkmcv3.so"
# 加密的master.ks
MASTER_KS = f"{get_install_head_path()}/DataBackup/ProtectClient/ProtectClient-E/conf/kmc_store.txt"
# 编码格式
ENCODING = "UTF-8"


class KmcStatus(int, Enum):
    KMC_SUCCESS = 0
    KMC_FAIL = 1
    KMC_ENCTXT_INVAILD = 2  # 无效密文错误码


def encode_text(text):
    if isinstance(text, bytes):
        return text
    else:
        return text.encode(ENCODING)


def decode_text(text):
    if isinstance(text, bytes):
        return text.decode(ENCODING)
    else:
        return text


def _build_kmc_handler():
    handler = ctypes.cdll.LoadLibrary(LIB_PATH)
    global GLOBAL_KMC_INIT_FLAG
    if GLOBAL_KMC_INIT_FLAG:
        de_init_ret = handler.DeInitKmc()
        if de_init_ret != KmcStatus.KMC_SUCCESS:
            return handler
        GLOBAL_KMC_INIT_FLAG = False
    if not os.path.exists(MASTER_KS):
        return handler
    init_ret = handler.InitKMCV3c(encode_text(MASTER_KS),
                                  encode_text(MASTER_KS),
                                  encode_text("GeneralDBPlugin"))
    if init_ret != KmcStatus.KMC_SUCCESS:
        return handler
    GLOBAL_KMC_INIT_FLAG = True
    return handler


class Kmc:
    _instance_lock = Lock()
    _instance = None
    _handler = None

    def __new__(cls, *args, **kwargs):
        if cls._instance is None:
            with cls._instance_lock:
                if cls._instance is None:
                    cls._instance = object.__new__(cls)
                    cls._handler = _build_kmc_handler()
        return cls._instance

    def encrypt(self, plain_text: str, domain_id=SecurityConstants.DEFAULT_DOMAIN_ID) -> str:
        """
        加密明文字符串
        :param plain_text: str 需要加密的明文字符串
        :param domain_id: int 加密使用的域ID
        :return: str 密文字符串
        """
        if not plain_text:
            return plain_text
        cipher = ctypes.c_char_p()
        try:
            p_cipher = ctypes.pointer(cipher)
            result = self._handler.EncryptV3c(domain_id, encode_text(plain_text), p_cipher)
            if result != KmcStatus.KMC_SUCCESS:
                raise Exception("kmc encrypt failed")
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
            return cipher_text
        plain = ctypes.c_char_p()
        try:
            p_plain = ctypes.pointer(plain)
            result = self._handler.DecryptV3c(domain_id, p_plain, encode_text(cipher_text))
            if result != KmcStatus.KMC_SUCCESS:
                raise Exception("kmc decrypt failed")
            plain_text = decode_text(plain.value)
        finally:
            # 失败底层会释放指针
            self._handler.KmcFree(plain)
        return str(plain_text).strip() if plain_text else plain_text
