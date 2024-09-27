#!/usr/bin/python

import ctypes
import os
import shlex
import subprocess
import threading


from kmc.consts import KmcConstant, KmcStatus

glb_init_flag = False
glb_kmc_lock = threading.Lock()
glb_default_domain_id = 0


class Kmcv3Wapper():

    def __init__(self, logger):
        self.kmc_handler = ctypes.cdll.LoadLibrary(KmcConstant.KMC_LIB_PATH)
        self.logger = logger

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

    def decrypt(self, cipher_text, domain_id=glb_default_domain_id):
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
