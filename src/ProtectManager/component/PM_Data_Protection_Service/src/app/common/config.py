# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
import os
import time
import urllib.parse
from typing import Optional, Dict, Any

import urllib3
from pydantic import BaseSettings, validator, PostgresDsn

from app.common import logger
from app.common.clients.client_util import InfrastructureHttpsClient, is_response_status_ok, parse_response_data, \
    SystemBaseHttpsClient
from app.common.clients.device_manager_client import device_manager_client

__all__ = ["settings"]

from app.common.exter_attack import exter_attack

log = logger.get_logger(__name__)


class Settings(BaseSettings):
    TIMEZONE = device_manager_client.init_time_zone()
    DPS_SERVICE_PORT = os.getenv("DPS_SERVICE_PORT", "30092")
    RM_SERVICE_PORT = os.getenv("RM_SERVICE_PORT", "30094")
    SCHEDULER_SERVICE_PORT = os.getenv("SCHEDULER_SERVICE_PORT", "30096")

    # Get database config
    POSTGRES_HOST: str = os.getenv("POSTGRES_HOST", "gaussdb")
    POSTGRES_PORT: str = os.getenv("POSTGRES_PORT", "6432")
    POSTGRES_USERNAME: str = os.getenv("POSTGRES_USERNAME", "generaldb")
    POSTGRES_SUPER_USERNAME: str = os.getenv("POSTGRES_SUPER_USERNAME", "gaussdbremote")
    POSTGRES_DB: str = os.getenv("POSTGRES_DB", "protect_manager")
    POSTGRES_ENCODING: str = os.getenv("POSTGRES_ENCODING", "utf8")
    POSTGRES_ECHO: bool = os.getenv("POSTGRES_ECHO", "False").lower() == "true"

    # Get database pool config
    DATABASE_MAX_OVERFLOW: int = int(os.getenv("DATABASE_MAX_OVERFLOW", "30"))
    DATABASE_POOL_SIZE: int = int(os.getenv("DATABASE_POOL_SIZE", "10"))
    DATABASE_POOL_TIMEOUT: int = int(os.getenv("DATABASE_POOL_TIMEOUT", "50"))
    DATABASE_POOL_RECYCLE: int = int(os.getenv("DATABASE_POOL_RECYCLE", "300"))
    DATABASE_POOL_PRE_PING: bool = os.getenv("DATABASE_POOL_PRE_PING", "True").lower() == "true"

    SERVICE_HOST: str = os.getenv("SERVICE_HOST", "0.0.0.0")
    SERVICE_PORT: int = int(os.getenv("SERVICE_PORT", "80"))
    BOOTSTRAP_SERVER: str = os.getenv("BOOTSTRAP_SERVER", "infrastructure-zk-kafka:9092")
    KAFKA_USERNAME: str = os.getenv("KAFKA_USERNAME", "kafka_usr")

    # Get redis config
    REDIS_HOST: str = os.getenv("REDIS_HOST", "infrastructure")
    REDIS_PORT: int = int(os.getenv("REDIS_PORT", "6369"))
    REDIS_SLAVE_PORT: int = int(os.getenv("REDIS_SLAVE_PORT", "6370"))
    REDIS_USERNAME: str = os.getenv("REDIS_USERNAME", "default")
    REDIS_DB: int = os.getenv("REDIS_DB", "0")
    REDIS_CLUSTER_NODES: list = [
        "infrastructure-0.infrastructure.dpa.svc.cluster.local",
        "infrastructure-1.infrastructure.dpa.svc.cluster.local",
        "infrastructure-2.infrastructure.dpa.svc.cluster.local"
    ]

    APPLICATION_NAME: str = os.getenv("APPLICATION_NAME", "")
    NODE_NAME: str = os.getenv("NODE_NAME")
    LOGGING_DIR: str = os.getenv("LOGGING_PATH", "/opt/OceanProtect/logs")
    LOGGING_PATH: str = None

    SLA_COUNT_LIMIT: int = 128

    REPLICATION_POLICY_COUNT_LIMIT: int = 4
    ARCHIVE_POLICY_COUNT_LIMIT: int = 4
    FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT: int = 1
    DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT: int = 1
    CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT: int = 1
    LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT: int = 1
    PERMANENT_INCREMENT_BACKUP_POLICY_COUNT_DEFAULT_LIMIT: int = 1

    @staticmethod
    def quote_plus_text(text, quote_plus=True):
        if not quote_plus:
            return text
        return urllib.parse.quote_plus(text)

    @staticmethod
    @exter_attack
    def get_password(env_key, key, quote_plus=False):
        try:
            response = InfrastructureHttpsClient().request(
                "GET", f"/v1/infra/secret/info?nameSpace=dpa&secretName=common-secret")
        except urllib3.exceptions.HTTPError:
            log.exception(f"get {key} exception")
            return ""
        if not is_response_status_ok(response):
            log.error(f"get {key} failed")
            return Settings.quote_plus_text(os.getenv(env_key, ""), quote_plus=quote_plus)
        for json_obj in parse_response_data(response.data).get("data", []):
            if json_obj.get(key):
                log.info(f"get {key} success")
                return Settings.quote_plus_text(json_obj.get(key), quote_plus=quote_plus)
        return Settings.quote_plus_text(os.getenv(env_key, ""), quote_plus=quote_plus)

    @staticmethod
    @exter_attack
    def get_key_from_config_map(config_map, config_key):
        url = f"/v1/infra/configmap/info?nameSpace=dpa&configMap={config_map}&configKey={config_key}"
        response = InfrastructureHttpsClient().request("GET", url)
        log.info(f"get key from configmap, response: {response.data}")
        if not is_response_status_ok(response):
            log.error(f"get infra/configmap/info error")
            return ""
        system_info = parse_response_data(response.data).get("data", [])
        if system_info:
            return system_info[0].get(config_key, "")
        log.error(f"parse response data error.")
        return ""

    @staticmethod
    @exter_attack
    def get_esn_from_sys_base():
        url = f"/v1/internal/clusters/esn"
        response = SystemBaseHttpsClient().request("GET", url)
        if not is_response_status_ok(response):
            log.error(f"get infra/configmap/info error")
            return ""
        log.info(f"get esn from sysbase, response: {response.data}")
        return response.data.decode("utf-8")

    @classmethod
    def get_db_password(cls) -> str:
        while True:
            temp_text = cls.get_password("POSTGRES_PASSWORD", "database.generalPassword", quote_plus=True)
            if temp_text != "":
                log.info("get database password success.")
                return temp_text
            log.warning("get database password failed.")
            time.sleep(5)

    @classmethod
    def get_redis_password(cls) -> str:
        while True:
            temp_text = cls.get_password("REDIS_PASSWORD", "redis.password")
            if temp_text != "":
                log.info("get redis password success.")
                return temp_text
            log.warning("get redis password failed.")
            time.sleep(5)

    @classmethod
    def get_kafka_password(cls) -> str:
        while True:
            temp_text = cls.get_password("KAFKA_PASSWORD", "kafka.password")
            if temp_text != "":
                log.info("get kafka password success.")
                return temp_text
            log.warning("get kafka password failed.")
            time.sleep(5)

    @validator("LOGGING_PATH", pre=True)
    def build_logging_path(cls, v: Optional[str], values: Dict[str, Any]) -> Any:
        node_name = os.getenv("NODE_NAME")
        logging_path = os.getenv("LOGGING_PATH", "/opt/OceanProtect/logs") + "/"
        if node_name:
            logging_path = logging_path + node_name + "/"
        logging_path += "protectmanager/"
        return logging_path


settings = Settings()
