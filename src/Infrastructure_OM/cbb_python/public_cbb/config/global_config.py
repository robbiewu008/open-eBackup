#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import threading
from typing import Optional
import functools
import os
import importlib
import time
import stat
from pydantic import BaseSettings
from public_cbb.log.logger import get_logger

logger = get_logger()
ENV_FILE_PATH = '/cache/.env'


def singleton(cls):
    _instance = {}
    count = 0

    @functools.wraps(cls)
    def _singleton(*args, **kargs):
        if kargs.get("no_deco"):
            return cls(*args, **kargs)
        nonlocal count
        if cls not in _instance:
            _instance.update({cls: cls(*args, **kargs)})
        count += 1
        return _instance.get(cls)

    return _singleton


class GlobalConfig(BaseSettings):
    """Global configurations."""

    INFRA_HOST: Optional[str] = "infrastructure.dpa.svc.cluster.local"
    INFRA_DB_HOST: Optional[str] = "gaussdb"
    INFRA_DB_PORT: Optional[str] = "6432"
    INFRA_HTTP_PORT: Optional[str] = "8088"

    OPEN_CERT_VERIFY: bool = True
    OPEN_DATABASE_VERIFY: bool = False

    AGENT_CA_DIR: str = "/opt/OceanProtect/protectmanager/cert/internal/ProtectAgent/ca/ca.crt.pem"

    EXTERNAL_CERT_DIR: str = "/opt/OceanProtect/protectmanager/cert/internal/OpenAPI/OpenAPI.crt.pem"
    EXTERNAL_KEY_DIR: str = "/opt/OceanProtect/protectmanager/cert/internal/OpenAPI/OpenAPI.pem"
    EXTERNAL_CA_DIR: str = "/opt/OceanProtect/protectmanager/cert/CA/certs/ca.crt.pem"
    EXTERNAL_CNF_DIR: str = "/opt/OceanProtect/protectmanager/cert/internal/OpenAPI/OpenAPI.cnf"
    EXTERNAL_CLR_DIR: str = "/opt/OceanProtect/protectmanager/cert/crl/ProtectAgent.crl"

    INTERNAL_CERT_DIR: str = "/opt/OceanProtect/infrastructure/cert/internal/internal.crt.pem"
    INTERNAL_KEY_DIR: str = "/opt/OceanProtect/infrastructure/cert/internal/internal.pem"
    INTERNAL_CA_DIR: str = "/opt/OceanProtect/infrastructure/cert/internal/ca/ca.crt.pem"
    INTERNAL_CNF_DIR: str = "/opt/OceanProtect/infrastructure/cert/internal/internal_cert"

    LIBKMCV3_SO_PATH: str = "/usr/lib64/libkmcv3.so"
    MASTER_KS_PATH: str = "/opt/OceanProtect/protectmanager/kmc/master.ks"
    BACKUP_KS_PATH: str = "/kmc_conf/..data/backup.ks"

    DATABASE_MAX_OVERFLOW: int = 30
    DATABASE_POOL_SIZE: int = 50
    DATABASE_POOL_TIMEOUT: int = 50
    DATABASE_POOL_RECYCLE: int = 600
    DATABASE_POOL_PRE_PING: bool = True

    NETWORK_CONFIG_PATH: str = "/opt/network_conf"
    ANNOTATIONS_CONFIG_PATH: str = '/opt/podinfo/annotations'

    MOUNT_OPER_PATH: str = '/usr/bin/mount_oper.sh'

    TIME_SCHEDULER_WORKS: int = 5
    MAX_CACHE_CONNECTION: int = 1
    HTTP_TIME_OUT: int = 120
    MAX_HTTP_RETRIES: int = 3

    FS_COMPRESS_ENABLE: bool = True
    FS_DEDUP_ENABLE: bool = True

    INFRA_HTTP_RETRY_TIMES: int = 10
    INFRA_HTTP_RETRY_INTERVAL: int = 6

    PM_HTTP_RETRY_TIMES: int = 60
    PM_HTTP_RETRY_INTERVAL: int = 10

    STATUS_QUERY_INTERVAL_SEC: int = 6  # unit: sec
    STATUS_QUERY_TIMEOUT_SEC: int = 14400  # query time: 24h

    DATA_REPOSITORY_DIST_ALG: int = 4  # CAPACITY_BALANCE_MODE = 1, CONTROLLER_AFFINITY_MODE = 4
    META_REPOSITORY_DIST_ALG: int = 4
    CACHE_REPOSITORY_DIST_ALG: int = 4
    LOG_REPOSITORY_DIST_ALG: int = 4
    INFRA_RETRY_TIMES_FOR_NETWORK_ERROR: int = 3
    INFRA_RETRY_INTERVAL_FOR_NETWORK_ERROR: int = 40

    KEEPALIVES_IDLE: int = 300   # unit: sec
    KEEPALIVES_INTERVAL: int = 30   # unit: sec
    KEEPALIVES_COUNT: int = 5
    TCP_USER_TIMEOUT: int = 150000  # unit: milli sec

    class Config:
        """Loads the dotenv file."""

        env_file = ENV_FILE_PATH
        case_sensitive = False


# singleton Class to ensure only have one watch thread
@singleton
class ConfigMgr:

    def __init__(self):
        self.config = GlobalConfig
        if os.getenv('DEPLOY_ENV') == 'k8s_cluster':
            self.init_success = False
            threading.Thread(target=self.watch_configmap_event, name='watch_configmap_event').start()
            while True:
                # init success when new dot env file generated
                if self.init_success:
                    break
                time.sleep(1)

    def register(self, app_config, env_file=".env"):
        self.config = app_config
        self.config.Config.env_file = env_file

    def watch_configmap_event(self):
        logger.info('Config map event watch thread is running.')
        # import k8s api module when deploy in k8s_cluster
        # non_k8s_cluster env doesn't need to do it
        k8s_base_module = importlib.import_module('public_cbb.kubernetes_api.base')
        k8s_api_cls = getattr(k8s_base_module, 'KubernetesApi')
        k8s_api_cls().register_watch(
            api_class_name='CoreV1Api',
            api_method_name='list_namespaced_config_map',
            handle_event_func=self.check_configmap_event,
            namespace='dpa',
            field_selector=f'metadata.name=dme-ubc-conf'
        )

    def check_configmap_event(self, event):
        data = event.get('object').data
        self.generate_dotenv_from_configmap(data)
        self.init_success = True

    def generate_dotenv_from_configmap(self, configmap_data):
        logger.info('Configmap changed, generate dotenv file from configmap.')
        # 'w' flag will clean the file when open it
        flags = os.O_WRONLY | os.O_CREAT
        modes = stat.S_IWUSR | stat.S_IRUSR
        with os.fdopen(os.open(self.config.Config.env_file, flags, modes), 'w') as f:
            for key, value in configmap_data.items():
                f.write(f'{key}={value}\n')
        logger.info(f'New dotenv file generated, file path:{self.config.Config.env_file}')


@functools.lru_cache()
def get_settings():
    return ConfigMgr().config()
