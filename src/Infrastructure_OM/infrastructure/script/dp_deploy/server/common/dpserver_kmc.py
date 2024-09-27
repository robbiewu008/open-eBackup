#!/usr/bin/python

import ctypes
import os
import shlex
import subprocess
import threading
from enum import Enum
import base64

from server.common.logger.logger import get_logger
from server.common import consts


class KmcStatus(int, Enum):
    KMC_SUCCESS = 0
    KMC_FAIL = 1
    KMC_ENCTXT_INVAILD = 2  # 无效密文错误码


glb_init_flag = False
glb_kmc_lock = threading.Lock()
GLB_DEFAULT_DOMAIN_ID = 0


class Kmcv3Wapper:
    log_file = consts.KMC_LOG_PATH
    os.makedirs(os.path.dirname(log_file), exist_ok=True)
    logger = get_logger(log_file)

    def __init__(self, lib_path=consts.KMC_LIB_PATH_DPSERVER):
        self.kmc_handler = ctypes.cdll.LoadLibrary(lib_path)

    @staticmethod
    def encode(value):
        if isinstance(value, bytes):
            return value
        else:
            return value.encode('UTF-8')

    @staticmethod
    def decode(value):
        if isinstance(value, bytes):
            return value.decode('UTF-8')
        else:
            return value

    def initialize(self, masterkey_file, backupkey_file, moudle_name):
        """
        初始化和去初始化需要成对出现 initialize---finalize
        :param masterkey_file: str 密钥文件路径
        :param backupkey_file: str 备份密钥文件路径
        :param moudle_name: str 使用模块名称，用于KMC底层日志记录
        :return: 失败 非0；成功 0
        """

        if masterkey_file is None or backupkey_file is None or moudle_name is None:
            self.logger.warning('KMC initialize param error, pls check.')
            return KmcStatus.KMC_FAIL

        global glb_init_flag
        if glb_init_flag:
            self.logger.warning('KMC already initialized.')
            return KmcStatus.KMC_SUCCESS

        try:
            glb_kmc_lock.acquire()
            if not os.path.exists(os.path.dirname(masterkey_file)):
                cmd = "mkdir -p %s" % os.path.dirname(masterkey_file)
                subprocess.Popen(shlex.split(cmd), shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            if not os.path.exists(os.path.dirname(backupkey_file)):
                cmd = "mkdir -p %s" % os.path.dirname(backupkey_file)
                subprocess.Popen(shlex.split(cmd), shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            result = self.kmc_handler.InitKMCV3c(Kmcv3Wapper.encode(masterkey_file),
                                                 Kmcv3Wapper.encode(backupkey_file),
                                                 Kmcv3Wapper.encode(moudle_name))
            if result == KmcStatus.KMC_SUCCESS:
                self.logger.info('KMC initialize success.')
                glb_init_flag = True
            else:
                self.logger.error('KMC initialize failed.')
                return KmcStatus.KMC_FAIL
        except Exception as e:
            self.logger.error('KMC initialize failed. Exception=%s' % str(e))
            return KmcStatus.KMC_FAIL
        finally:
            glb_kmc_lock.release()

        return KmcStatus.KMC_SUCCESS

    def finalize(self):
        """
        去初始化和去初始化需要成对出现 initialize---finalize
        :return:  失败 非0；成功 0
        """
        try:
            glb_kmc_lock.acquire()
            result = self.kmc_handler.DeInitKmc()
            global glb_init_flag
            glb_init_flag = False
            if result == KmcStatus.KMC_SUCCESS:
                self.logger.info('KMC finalize success.')
            else:
                self.logger.error('KMC finalize failed.')
                return KmcStatus.KMC_FAIL
        except Exception as e:
            self.logger.error('KMC finalize failed. Exception=%s' % str(e))
            return KmcStatus.KMC_FAIL
        finally:
            glb_kmc_lock.release()

        return KmcStatus.KMC_SUCCESS

    def encrypt(self, plain_text, domain_id=GLB_DEFAULT_DOMAIN_ID):
        """
        加密字符串
        :param plain_text: str 需要加密字符串
        :param domain_id: int 加密使用的域ID
        :return: (retcode, cipherstr) retcode==0, 成功，cipherstr为加密字符串；retcode!=0, 成功，cipherstr为None；
        """
        if not glb_init_flag:
            self.logger.error('KMC not initialize.')
            return KmcStatus.KMC_FAIL, ""

        if plain_text is None:
            self.logger.error('param plain_text is None.')
            return KmcStatus.KMC_FAIL, ""

        cipher_text = None
        cipher = ctypes.c_char_p()
        p_cipher = ctypes.pointer(cipher)
        result = self.kmc_handler.EncryptV3c(domain_id, Kmcv3Wapper.encode(plain_text), p_cipher)
        if result == KmcStatus.KMC_SUCCESS:
            self.logger.info('KMC encrypt success.')
            cipher_text = Kmcv3Wapper.decode(cipher.value)
            self.kmc_handler.KmcFree(cipher)  # 失败底层会释放指针
        else:
            self.logger.error('KMC encrypt failed. err_code=%s' % str(result))
            return result, ""

        return result, cipher_text

    def decrypt(self, cipher_text, domain_id=GLB_DEFAULT_DOMAIN_ID):
        """
        解密字符串
        :param cipher_text: str 需要解密字符串
        :param domain_id: int 解密使用的域ID
        :return: (retcode, plainstr) retcode==0, 成功，plainstr为解密字符串；retcode!=0, 成功，plainstr为None；
        """
        if not glb_init_flag:
            self.logger.error('KMC not initialize.')
            return KmcStatus.KMC_FAIL, ""

        if cipher_text is None:
            self.logger.error('param cipher_text is None.')
            return KmcStatus.KMC_FAIL, ""

        plain_text = None
        plain = ctypes.c_char_p()
        p_plain = ctypes.pointer(plain)
        result = self.kmc_handler.DecryptV3c(domain_id, p_plain, Kmcv3Wapper.encode(cipher_text))
        if result == KmcStatus.KMC_SUCCESS:
            self.logger.info('KMC decrypt success.')
            plain_text = Kmcv3Wapper.decode(plain.value)
            self.kmc_handler.KmcFree(plain)  # 失败底层会释放指针
        else:
            self.logger.error('KMC decrypt failed. err_code=%s' % str(result))
            return result, ""

        return result, plain_text


def encode_base64(data):
    """
    将data转化为base64编码
    :param data:
    :return:
    """
    encoded_bytes = base64.b64encode(data.encode("utf-8"))
    return str(encoded_bytes, "utf-8")


def decode_base64(data):
    """
    将data进行base64解码
    :param data:
    :return:
    """
    decoded_bytes = base64.b64decode(data.encode("utf-8"))
    return str(decoded_bytes, "utf-8")
