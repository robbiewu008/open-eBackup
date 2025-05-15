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
from dataclasses import dataclass, field
from typing import Dict, Any

from app.common import logger

__all__ = ['config']

from app.common.config import settings
from app.common.constants.constant import SecurityConstants

log = logger.get_logger(__name__)


@dataclass
class KafkaConfig:
    bootstrap_server: str = 'kafka-broker:9092'
    admin_conf: Dict[str, Any] = field(default_factory=dict)
    producer_conf: Dict[str, Any] = field(default_factory=dict)
    consumer_conf: Dict[str, Any] = field(default_factory=dict)


config = KafkaConfig()
kafka_password = settings.get_kafka_password()

# admin
config.admin_conf['bootstrap.servers'] = config.bootstrap_server
config.admin_conf["security.protocol"] = "SASL_SSL"
config.admin_conf["sasl.mechanism"] = "PLAIN"
config.admin_conf["sasl.username"] = settings.KAFKA_USERNAME
config.admin_conf["sasl.password"] = kafka_password
config.admin_conf["ssl.ca.location"] = SecurityConstants.INTERNAL_CA_FILE

# producer
config.producer_conf['bootstrap.servers'] = config.bootstrap_server
config.producer_conf["security.protocol"] = "SASL_SSL"
config.producer_conf["sasl.mechanism"] = "PLAIN"
config.producer_conf["sasl.username"] = settings.KAFKA_USERNAME
config.producer_conf["sasl.password"] = kafka_password
config.producer_conf["ssl.ca.location"] = SecurityConstants.INTERNAL_CA_FILE

# consumer
config.consumer_conf['bootstrap.servers'] = config.bootstrap_server
config.consumer_conf["security.protocol"] = "SASL_SSL"
config.consumer_conf["sasl.mechanism"] = "PLAIN"
config.consumer_conf["sasl.username"] = settings.KAFKA_USERNAME
config.consumer_conf["sasl.password"] = kafka_password
config.consumer_conf["ssl.ca.location"] = SecurityConstants.INTERNAL_CA_FILE
config.consumer_conf['group.id'] = 'backup_manager'
config.consumer_conf['session.timeout.ms'] = 6000
config.consumer_conf['auto.offset.reset'] = 'earliest'
config.consumer_conf['enable.auto.commit'] = False
