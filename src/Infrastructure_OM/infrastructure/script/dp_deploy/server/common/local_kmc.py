#!/usr/bin/env python
# -*-coding:utf-8-*-
import os
import sys
import pwd
import grp
import ctypes
from ctypes import *
import logging.handlers
from server.common import consts

if 2 == sys.version_info[0]:
    python_version = 2
else:
    python_version = 3

try:
    import ConfigParser as ConfigParser
except:
    import configparser as ConfigParser

# 初始化日志，1个备份，最大1MB，忽略异常，不影响脚本执行
logfile = consts.KMC_LOG_PATH
os.umask(0o0027)  # 设置日志文件权限

try:
    logger = logging.getLogger('')
    logger.setLevel(logging.DEBUG)  # 脚本频繁调用，不打info以下级别日志
    handler = logging.handlers.RotatingFileHandler(logfile, maxBytes=1024 * 1024, backupCount=1)
    formatter = logging.Formatter(fmt='[%(asctime)s] [%(filename)s:%(lineno)d] [%(levelname)s]: %(message)s',
                                  datefmt="%Y-%m-%d %H:%M:%S")
    handler.setFormatter(formatter)
    logger.addHandler(handler)

    usrOmmdba = pwd.getpwnam('omm')
    grpOmm = grp.getgrnam('oam')
    os.lchown(logfile, usrOmmdba.pw_uid, grpOmm.gr_gid)  # 设置日志文件属主
except Exception:
    pass

# 全局变量定义
KMCA_DOMAIN_ID_KMC_0 = 0
KMCA_DOMAIN_ID_KMC_2 = 2
KMCA_DOMAIN_ID_KMC_50 = 50

KMC_LIB = os.path.join(consts.DPSERVER_KMC, consts.KMC_LIB_NAME)

KMC_PUB_PRI_FILE = b'/opt/dfv/oam/public/kmc/kmca_primary_file.dat'
KMC_PUB_SBY_FILE = b'/opt/dfv/oam/public/kmc/kmca_standby_file.dat'

# 注册日志回调函数，日志级别，详见KMCA_LOG_TYPE
KMCA_LOG_EMERG = 1
KMCA_LOG_ERROR = 2
KMCA_LOG_WARNING = 3
KMCA_LOG_INFO = 4
KMCA_LOG_DEBUG = 5


def kmc_logger(level, log):
    if level == KMCA_LOG_EMERG or level == KMCA_LOG_ERROR:
        logger.error('%s', log)
    elif level == KMCA_LOG_WARNING:
        logger.warning('%s', log)
    elif level == KMCA_LOG_INFO:
        logger.info('%s', log)
    elif level == KMCA_LOG_DEBUG:
        logger.debug('%s', log)
    else:
        logger.debug('%s', log)


_logCbFunc = CFUNCTYPE(None, c_uint, c_char_p)
kmcLogFunc = _logCbFunc(kmc_logger)


# 调用kmc动态库加解密
class KmcApi(object):
    _loaded = False
    _instance = None

    def __new__(cls, *args, **kw):
        if cls._instance is None:
            cls._instance = super(KmcApi, cls).__new__(cls)
        return cls._instance

    def __init__(self, dllfile):
        self.path = dllfile

    def _initial(self, logfunc, prifile, sdyfile):
        try:
            self._dllclose = ctypes.cdll.LoadLibrary('').dlclose
            self._dllclose.argtypes = [c_void_p]
            self.lib = ctypes.cdll.LoadLibrary(self.path)
            self._handle = self.lib._handle
        except Exception as err:
            logger.error('load dll (%s) fail, err (%s).', self.path, str(err))
            return False
        ret = self.lib.KMCA_PyInit(logfunc, prifile, sdyfile)
        if ret != 0:
            logger.error('initial kmc (%s) fail, err (%d)', prifile, ret)
            return False
        self.prifile = prifile
        KmcApi._loaded = True
        return True

    def _finalize(self):
        self.lib.KMCA_Finalize.restype = c_int
        ret = self.lib.KMCA_Finalize()  # 释放密钥文件资源
        if ret != 0:
            logger.error('finalize kmc (%s) fail, err (%d)', self.prifile, ret)
            return False
        self._dllclose(self._handle)
        del self.lib  # 释放动态库资源
        KmcApi._loaded = False
        return True

    def initial(self, logfunc, prifile, sdyfile):
        if KmcApi._loaded:
            self._finalize()
        return self._initial(logfunc, prifile, sdyfile)

    def _decrypt(self, domainid, ciphertext, ciphertextlen, plaintext, plaintextlen):
        # 描述接口(参数都是ctypes类型，类似C语言接口用法)
        self.lib.KMCA_PyDecrypt.argtypes = [c_int, c_char_p, c_uint, c_char_p, POINTER(c_uint)]
        self.lib.KMCA_PyDecrypt.restype = c_int
        # 调用接口
        retcode = self.lib.KMCA_PyDecrypt(domainid, ciphertext, ciphertextlen, plaintext, pointer(plaintextlen))
        if retcode != 0:
            return False
        return True

    def decrypt(self, domainid, ciphertext):
        # 参数都是字符串类型，Python语言用法
        domainId = c_int(domainid)
        cipherBuffLen = 1024
        cipherText = create_string_buffer(cipherBuffLen)
        if len(ciphertext) >= cipherBuffLen:
            logger.error('cipher text len (%u) too long.', len(ciphertext))
            return False, None
        if python_version == 2:
            ctypes.memmove(cipherText, bytes(ciphertext), len(ciphertext))  # 给字节buffer赋值
        else:
            ctypes.memmove(cipherText, bytes(ciphertext, encoding='utf8'), len(ciphertext))  # 给字节buffer赋值
        cipherTextLen = c_uint(len(ciphertext))
        plainBuffLen = 1024
        plainText = create_string_buffer(plainBuffLen)
        plainTextLen = c_uint(plainBuffLen)
        result = self._decrypt(domainId, cipherText, cipherTextLen, plainText, plainTextLen)
        if not result:
            return False, None
        return True, plainText.value


def decrypt_private_key_passwd(private_passwd_path):
    kmc_api = KmcApi(KMC_LIB)
    ret_ok = kmc_api.initial(kmcLogFunc, KMC_PUB_PRI_FILE, KMC_PUB_SBY_FILE)
    with open(private_passwd_path) as file:
        passwd = file.read()
    a = kmc_api.decrypt(KMCA_DOMAIN_ID_KMC_2, passwd)
    pass_decrypt = a[1].decode('utf-8')
    return pass_decrypt
