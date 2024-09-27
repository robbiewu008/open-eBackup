#!/usr/bin/python
import base64
import ctypes
import os
import shlex
import subprocess
import threading
import logging

glb_init_flag = False
glb_kmc_lock = threading.Lock()
GLB_DEFAULT_DOMAIN_ID = 0

KMC_SUCCESS = 0
KMC_FAIL = 1
KMC_LIB_PATH = "/usr/lib64/libkmcv3.so"
KS_BAK_NAME = "backup.ks"
STORE_FILE = "/opt/OceanProtect/protectmanager/kmc/master.ks"
STORE_FILE_BAK = f"/kmc_conf/..data/{KS_BAK_NAME}"
MODULE_NAME = "infrastructure"


class KMC:
    def __init__(self):
        self.kmc_handler = ctypes.cdll.LoadLibrary(KMC_LIB_PATH)

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
            logging.warning('KMC initialize param error, pls check.')
            return KMC_FAIL

        global glb_init_flag
        if glb_init_flag:
            logging.warning('KMC already initialized.')
            return KMC_SUCCESS

        try:
            glb_kmc_lock.acquire()
            if not os.path.exists(os.path.dirname(masterkey_file)):
                cmd = "mkdir -p %s" % os.path.dirname(masterkey_file)
                subprocess.Popen(shlex.split(cmd), shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            if not os.path.exists(os.path.dirname(backupkey_file)):
                cmd = "mkdir -p %s" % os.path.dirname(backupkey_file)
                subprocess.Popen(shlex.split(cmd), shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            result = self.kmc_handler.InitKMCV3c(KMC.encode(masterkey_file),
                                                 KMC.encode(backupkey_file),
                                                 KMC.encode(moudle_name))
            if result == KMC_SUCCESS:
                logging.info('KMC initialize success.')
                glb_init_flag = True
            else:
                logging.error('KMC initialize failed.')
                return KMC_FAIL
        except Exception as e:
            logging.error('KMC initialize failed. Exception=%s' % str(e))
            return KMC_FAIL
        finally:
            glb_kmc_lock.release()

        return KMC_SUCCESS

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
            if result == KMC_SUCCESS:
                logging.info('KMC finalize success.')
            else:
                logging.error('KMC finalize failed.')
                return KMC_FAIL
        except Exception as e:
            logging.error('KMC finalize failed. Exception=%s' % str(e))
            return KMC_FAIL
        finally:
            glb_kmc_lock.release()

        return KMC_SUCCESS

    def encrypt(self, plain_text, domain_id=GLB_DEFAULT_DOMAIN_ID):
        """
        加密字符串
        :param plain_text: str 需要加密字符串
        :param domain_id: int 加密使用的域ID
        :return: (retcode, cipherstr) retcode==0, 成功，cipherstr为加密字符串；retcode!=0, 成功，cipherstr为None；
        """
        if not glb_init_flag:
            logging.error('KMC not initialize.')
            return KMC_FAIL, ""

        if plain_text is None:
            logging.error('param plain_text is None.')
            return KMC_FAIL, ""

        cipher_text = None
        cipher = ctypes.c_char_p()
        p_cipher = ctypes.pointer(cipher)
        result = self.kmc_handler.EncryptV3c(domain_id, KMC.encode(plain_text), p_cipher)
        if result == KMC_SUCCESS:
            logging.info('KMC encrypt success.')
            cipher_text = KMC.decode(cipher.value)
            self.kmc_handler.KmcFree(cipher)  # 失败底层会释放指针
        else:
            logging.error('KMC encrypt failed. err_code=%s' % str(result))
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
            logging.error('KMC not initialize.')
            return KMC_FAIL, ""

        if cipher_text is None:
            logging.error('param cipher_text is None.')
            return KMC_FAIL, ""

        plain_text = None
        plain = ctypes.c_char_p()
        p_plain = ctypes.pointer(plain)
        result = self.kmc_handler.DecryptV3c(domain_id, p_plain, KMC.encode(cipher_text))
        if result == KMC_SUCCESS:
            logging.info('KMC decrypt success.')
            plain_text = KMC.decode(plain.value)
            self.kmc_handler.KmcFree(plain)  # 失败底层会释放指针
        else:
            logging.error('KMC decrypt failed. err_code=%s' % str(result))
            return result, ""

        return result, plain_text


def encrypt_secret(plain_text):
    """
    加密字符串
    :param plain_text: str 需要加密字符串
    :return: 成功，返回加密后字符串；失败，抛出异常
    """
    kmc = KMC()
    status, cipher_text = kmc.encrypt(plain_text)
    if status == KMC_SUCCESS:
        return encode_base64(cipher_text)
    elif status == KMC_FAIL:
        kmc.finalize()
        ret_bool = kmc.initialize(STORE_FILE, STORE_FILE_BAK,
                                  MODULE_NAME)
        if ret_bool != KMC_SUCCESS:
            logging.error("KMC attempt to reinitialize failed!")
        status, cipher_text = kmc.encrypt(plain_text)
        if status != KMC_SUCCESS:
            logging.error("KMC decrypt failed!")
        return encode_base64(cipher_text)
    else:
        return encode_base64(cipher_text)


def decrypt_secret(cipher_text):
    """
    解密字符串
    :param cipher_text: str 需要解密字符串
    :return: 成功，返回解密后字符串；失败，1. 如果kmc解析无效，则返回原文 2. 重新初始化或解密失败，抛出异常
    """
    kmc = KMC()
    status, plain_text = kmc.decrypt(decode_base64(cipher_text))
    if status == KMC_SUCCESS:
        return plain_text
    elif status == KMC_FAIL:
        kmc.finalize()
        ret_bool = kmc.initialize(STORE_FILE, STORE_FILE_BAK,
                                  MODULE_NAME)
        if ret_bool != KMC_SUCCESS:
            logging.error("KMC attempt to reinitialize failed!")
        status, plain_text = kmc.decrypt(decode_base64(cipher_text))
        if status != KMC_SUCCESS:
            logging.error("KMC decrypt failed!")
        return plain_text
    else:
        return plain_text


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