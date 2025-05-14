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
from app.common.config import settings
from app.common.constants.constant import SecurityConstants

REDIS_DB = 0
REDIS_ABORT_KEY_ENTRY = 'requests_aborted'

# Default Consume config
default_consumer_config = {
    "bootstrap.servers": settings.BOOTSTRAP_SERVER,
    "security.protocol": "SASL_SSL",
    "sasl.mechanism": "PLAIN",
    "sasl.username": settings.KAFKA_USERNAME,
    "sasl.password": settings.get_kafka_password(),
    "ssl.ca.location": SecurityConstants.INTERNAL_CA_FILE,
    'session.timeout.ms': 20000,
    'max.poll.interval.ms': 1800000,
    'enable.auto.commit': False,
}


def get_default_consumer_config(group, **kwargs):
    """
    Create default configuration for kafka consumer object
    :param: group_id (str)  consumer group name
    """

    return {**kwargs, **default_consumer_config, "group.id": group}
