LOG_PATH = "/home/dpserver/log"
HOME_PACKAGE_PATH = '/home/package'
PACKAGE_PATH = '/opt/DataBackup/packages'
# 手动把所有nfs挂载到这个目录下，其实data_nas就可以了（初始失败后 gaussdb 密码变化问题）
MOUNT_PATH = '/opt/DataBackup'
SCRIPTS_PATH = '/usr/local/dpserver/scripts'
DPSERVER_PATH = '/usr/local/dpserver'
VERIFY_CHECKSUM_CMD = '{verify_tool} -f CHECK_PKG_INTEGRITY -e {check_sum} -h {cms} -c {crl}'
CHECK_SUM_FILE_NAME = 'sha256sum_sync'
CMS_FILE_NAME = '{}.cms'
CRL_FILE_NAME = 'crldata.crl'
CBB_VERIFY_TOOL = '/usr/bin/verify_tool'
VERIFY_TOOL_LIST = ["./sha256sum_sync", "./sha256sum_sync.cms", "./crldata.crl"]

MAX_SINGLE_UPLOAD = 8192  # 单次最大上传8kb
GET_NAMESPACE_NOT_EXIST = 33564678  # namespace不存在的时候返回的code

COMMAND_SUCCESS = 0
COMMAND_FAILED = 1
COMMAND_WRONG = 2

EXTENSION_LIST = ['tgz', 'gz']

MOUNT_CERT = '/opt/DataBackup/dpserver/cert'
PACIFIC_CERT = '/opt/fusionstorage/device-manager/nginx/conf/cert'

DPSERVER_CERT_BASE = '/home/dpserver/cert'
DPSERVER_CERT = f'{DPSERVER_CERT_BASE}/pacific'

PACIFIC_KMC = '/opt/dsware/tools/kmc/lib'
MOUNT_KMC = '/opt/DataBackup/dpserver/kmc'
DPSERVER_KMC = '/home/dpserver/kmc'
KMC_LOG_PATH = '/home/dpserver/log/kmc.log'

KMC_LIB_NAME = 'libomm_kmca.so'

SSL_KEYFILE_PATH = f'{DPSERVER_CERT}/device_manager_cipher.key'
SSL_CERTFILE_PATH = f'{DPSERVER_CERT}/device_manager.crt'
SSL_KEYFILE_PASSWORD_UNDECRYPT = f'{DPSERVER_CERT}/device_manager_cipher_password'

SSL_KEYFILE_PATH_DPSERVER = f'{DPSERVER_CERT_BASE}/device_manager_cipher.pem'
SSL_CERTFILE_PATH_DPSERVER = f'{DPSERVER_CERT_BASE}/device_manager.pem'
SSL_CA_CERTFLE_PATH_DPSERVER = f'{DPSERVER_CERT_BASE}/ca.pem'
SSL_KEYFILE_PASSWORD_UNDECRYPT_DPSERVER = f'{DPSERVER_CERT_BASE}/device_manager_cipher_password'
SSL_CIPHER = 'TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:DHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-SHA384:DHE-RSA-AES256-SHA256:ECDHE-ECDSA-AES128-SHA256:DHE-RSA-AES128-SHA256:RSA-PSK-AES256-GCM-SHA384:DHE-PSK-AES256-GCM-SHA384:RSA-PSK-CHACHA20-POLY1305:DHE-PSK-CHACHA20-POLY1305:ECDHE-PSK-CHACHA20-POLY1305:PSK-AES256-GCM-SHA384:PSK-CHACHA20-POLY1305:RSA-PSK-AES128-GCM-SHA256:DHE-PSK-AES128-GCM-SHA256:PSK-AES128-GCM-SHA256'

SLEEP_TIME = 20
DPSERVER_PORT = 25088

# upgrade
DPSERVER_BACKUP_PATH = '/opt/DataBackup/dpserver/backup'

DATAPROTECT_INSTALL_WAIT_TIMEOUT = 1800

PACIFIC_NETWORK_CONFIG = '/opt/network/network_config.ini'

SSL_KEYFILE_PASSWORD_UNDECRYPT_PACIFIC = f'{PACIFIC_CERT}/device_manager_cipher_password'

KMC_LIB_PATH_PACIFIC = f'{PACIFIC_KMC}/{KMC_LIB_NAME}'
KMC_LIB_PATH_MOUNT = f'{MOUNT_KMC}/{KMC_LIB_NAME}'
KMC_LIB_PATH_DPSERVER = f'{DPSERVER_CERT}/{KMC_LIB_NAME}'

CERT_VALIDITY_PERIOD = 3650
SSH_DEFAULT_PORT = 22

KMC_LIB_PATH_DPSERVER_DATABACKUP = f'{DPSERVER_PATH}/kmc/libkmcv3.so'
KMC_MASTER_KEY_FILE = f'{DPSERVER_PATH}/kmc/master.ks'
KMC_BACKUP_KEY_FILE = f'{DPSERVER_PATH}/kmc/backup.ks'

MICRO_SERVER_CERT_TAR_PATH = "/opt/DataBackup/cert.tgz"
LOCAL_PV_BASE_PATH = "/opt/DataBackup"
