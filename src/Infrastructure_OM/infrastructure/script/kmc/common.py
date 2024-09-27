import ctypes
import sys
import time

from kmc.consts import KmcStatus, KmcConstant, KmcError
from kmc.kmcv3_wapper import Kmcv3Wapper


def release_str_memory(sensitive_str):
    """
    释放敏感字符串内存，已确认，调用方法出的传参也可以释放内存
    :param sensitive_str: 敏感信息对象，
    :return: 无
    """
    buf_size = len(sensitive_str) + 1
    offset = sys.getsizeof(sensitive_str) - buf_size
    ctypes.memset(id(sensitive_str) + offset, 0, buf_size)


def initialize_kmc(master_ks: str, backup_ks: str, logger):
    """
    kmc工具初始化，最多尝试初始化10次，失败，抛出异常
    """
    kmc = Kmcv3Wapper(logger)
    for i in range(KmcConstant.KMC_INITIALIZATION_TIMES):
        ret_bool = kmc.initialize(master_ks, backup_ks,
                                  KmcConstant.MODULE_NAME)
        if ret_bool == KmcStatus.KMC_SUCCESS:
            return
        time.sleep(5)
    raise KmcError("KMC initialization error!")


def kmc_decrypt(code_file, logger):
    """
    解密路径文件
    :param code_file: str 密钥口令文件路径
    :param logger:
    :return: str 口令
    """
    with open(code_file, 'r') as encrypt_code:
        cipher_text = encrypt_code.read()
    kmc = Kmcv3Wapper(logger)
    return_code, plain_text = kmc.decrypt(cipher_text)
    if return_code != 0:
        raise Exception("Failed to decode cipher in kmc.")
    return plain_text
