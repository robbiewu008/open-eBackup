# coding: utf-8
from enum import Enum


class K8sConst:
    NAMESPACE = "dpa"
    COMMON_SECRET = "common-secret"
    PORT = 9888
    COMMON_CONF = "common-conf"


class HttpConst:
    PARAM_ERROR = 400
    OK = 200


class CustomErrorCode(object):
    """sftp服务管理自定义错误码"""
    ssh_config_modify_failed = "X001"
    ssh_config_not_exist = "X002"
    selinux_config_modify_failed = "X003"
    syslogd_config_modify_failed = "X005"
    get_secret_failed = "X007"
    get_config_map_failed = "X008"
    restart_sftp_service_failed = "X009"
    generate_rsa_key_failed = "X010"
    generate_ecdsa_key_failed = "X011"
    generate_ed25519_key_failed = "X012"
    pwd_ruler_config_modify_failed = "X013"
    pwd_ruler_config_not_exist = "X014"
    user_history_pwd_delete_failed = "X015"
    user_history_pwd_not_exist = "X016"
    invalid_os_pwd = "400131"
    delete_nonexistent_user = "400141"
    umount_failed = "400151"
    ssh_process_not_exist = "400161"


class SftpConst:
    SSHD_CONFIG = "/etc/ssh/sshd_config"
    SFTP_GROUP_NAME = "sftp"
    SYSLOGD_CONFIG = "/etc/rsyslog.d/sftpchroot.conf"
    PWD_TEMP_SAVE_DATA = "/etc/security/opasswd"
    PWD_RULER_SYSTEM_AUTH_LOCAL = "/etc/pam.d/system-auth-local"
    PWD_RULER_PASSWORD_AUTH_LOCAL = "/etc/pam.d/password-auth-local"
    SFTP_IP = "sftp_ip"


class KmcConstant:
    STORE_FILE = "/opt/OceanProtect/protectmanager/kmc/master.ks"
    STORE_FILE_BAK = "/kmc_conf/..data/backup.ks"
    CA_FILE = "/opt/OceanProtect/infrastructure/cert/internal/ca/ca.crt.pem"
    CERT_FILE = "/opt/OceanProtect/infrastructure/cert/internal/internal.crt.pem"
    KEY_FILE = "/opt/OceanProtect/infrastructure/cert/internal/internal.pem"
    INFRA_CERT = "/opt/OceanProtect/infrastructure/cert/internal/internal_cert"
    KMC_INITIALIZATION_TIMES = 10  # kmc初始化重试次数
    KMC_LIB_PATH = "/usr/lib64/libkmcv3.so"  # 依赖c/c++代码动态库
    MODULE_NAME = "infrastructure"


class KmcStatus(int, Enum):
    KMC_SUCCESS = 0
    KMC_FAIL = 1
    KMC_ENCTXT_INVAILD = 2  # 无效密文错误码


class KmcError(RuntimeError):
    def __init__(self, arg):
        self.args = arg


class InfraUrlConstant:
    INFRASTRUCTURE = "https://infrastructure.dpa.svc.cluster.local:8088"
    GET_SECRET = f"{INFRASTRUCTURE}/v1/infra/secret/info"
    GET_CONFIG = f"{INFRASTRUCTURE}/v1/infra/configmap/info"
