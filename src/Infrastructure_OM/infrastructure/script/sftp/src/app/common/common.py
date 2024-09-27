# coding: utf-8
import ctypes
import ipaddress
import json
import ssl
import sys
import time
from urllib import request

from app.common.const import KmcConstant, KmcStatus, KmcError
from app.common.kmcv3_wapper import Kmcv3Wapper


def check_ip_address(ip_addr: str) -> bool:
    """
    检查是否是IP地址
    :param ip_addr: IP地址
    :return: True:是，False:否
    """
    try:
        return bool(ipaddress.ip_address(ip_addr))
    except ValueError:
        return False


def release_str_memory(sensitive_str):
    """
    释放敏感字符串内存，已确认，调用方法出的传参也可以释放内存
    :param sensitive_str: 敏感信息对象，
    :return: 无
    """
    buf_size = len(sensitive_str) + 1
    offset = sys.getsizeof(sensitive_str) - buf_size
    ctypes.memset(id(sensitive_str) + offset, 0, buf_size)


def initialize_kmc():
    """
    kmc工具初始化，最多尝试初始化10次，失败，抛出异常
    """
    kmc = Kmcv3Wapper()
    for i in range(KmcConstant.KMC_INITIALIZATION_TIMES):
        ret_bool = kmc.initialize(KmcConstant.STORE_FILE, KmcConstant.STORE_FILE_BAK,
                                  KmcConstant.MODULE_NAME)
        if ret_bool == KmcStatus.KMC_SUCCESS:
            return
        time.sleep(5)
    raise KmcError("KMC initialization error!")


def kmc_decrypt(code_file):
    """
    解密路径文件
    :param code_file: str 密钥口令文件路径
    :param logger:
    :return: str 口令
    """
    with open(code_file, 'r') as encrypt_code:
        cipher_text = encrypt_code.read()
    kmc = Kmcv3Wapper()
    return_code, plain_text = kmc.decrypt(cipher_text)
    if return_code != 0:
        raise Exception("Failed to decode cipher in kmc.")
    return plain_text


def get_data_from_api(url, method, req_data=None):
    """
    支持https连接
    :param url: 请求路径，data：请求参数
    :return: 请求返会内容
    """
    context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
    context.check_hostname = False
    context.load_cert_chain(
        certfile=KmcConstant.CERT_FILE,
        keyfile=KmcConstant.KEY_FILE,
        password=kmc_decrypt(KmcConstant.INFRA_CERT)
    )
    context.load_verify_locations(cafile=KmcConstant.CA_FILE)
    context.verify_mode = ssl.CERT_REQUIRED
    params = json.dumps(req_data)
    params = params.encode()
    headers = {"Content-Type": "application/json"}
    req = request.Request(url, data=params, method=method, headers=headers)
    res = request.urlopen(req, context=context)
    return res
