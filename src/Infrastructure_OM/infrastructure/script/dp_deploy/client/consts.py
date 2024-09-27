DATAPROTECT_SERVER_POST = "25088"
PACIFIC_PORT = "8088"
BASE_NFS_LIST = [
    'global_image_repo',
    'nfs_simbaos',
    'nfs_simbaos_log',
    'nfs_simbaos_db',
]

OP_NFS_LIST = [
    "db-block",
    "comm-nas",
    "dee-nas",
    "inf-reserve",
    "data-nas",
    "agent-nas",
    "dme-nas",
    "pm-nas",
    "pm-nas2",
    "pm-nas3",
    "pm-report"
]

NFS_SIMBAOS = "nfs_simbaos"
NFS_GLOBAL = "global_image_repo"
STORAGE_POOL_NAME = 'DataBackup'
STORAGE_ACCOUNT_NAME = 'system'
SUPPORTED_PACAKAGE_TYPE = ['.tgz', '.tar.gz']

PACIFIC_TOKEN_KEEPALIVE_INTERNAL = 60 * 10

DPSERVER_UPGRADE_TIMEOUT = 5 * 60
DPSERVER_UPGRADE_WAIT_INTERNAL = 10

STORAGE_POOL_MAIN_STORAGE_DISK_TYPE = 'sata_disk'
STORAGE_POOL_CACHE_DISK_TYPE = 'ssd_card'
