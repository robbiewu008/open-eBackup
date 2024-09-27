from enum import Enum


class KmcStatus(int, Enum):
    KMC_SUCCESS = 0
    KMC_FAIL = 1
    KMC_ENCTXT_INVAILD = 2  # 无效密文错误码


class KmcConstant:
    KMC_INITIALIZATION_TIMES = 10  # kmc初始化重试次数
    KMC_LIB_PATH = "/usr/lib64/libkmcv3.so"  # 依赖c/c++代码动态库
    MODULE_NAME = "infrastructure"


class KmcError(RuntimeError):
    def __init__(self, arg):
        self.args = arg
