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
import redis
from redis import SSLConnection
from redis.cluster import ClusterNode

from app.common.config import settings
from app.common.constants.constant import SecurityConstants, ServiceConstants

is_redis_cluster_enabled = settings.get_key_from_config_map(ServiceConstants.MULTI_CLUSTER_CONF,
                                                            ServiceConstants.REDIS_CLUSTER)

if is_redis_cluster_enabled:
    startup_master_nodes = [ClusterNode(host, settings.REDIS_PORT) for host in settings.REDIS_CLUSTER_NODES]
    startup_slave_nodes = [ClusterNode(host, settings.REDIS_SLAVE_PORT) for host in settings.REDIS_CLUSTER_NODES]
    startup_nodes = []
    startup_nodes.extend(startup_master_nodes)
    startup_nodes.extend(startup_slave_nodes)

    redis_session = redis.RedisCluster(
        startup_nodes=startup_nodes,
        ssl=True, decode_responses=True, password=settings.get_redis_password(),
        ssl_keyfile=SecurityConstants.REDIS_KEY_FILE,
        ssl_certfile=SecurityConstants.REDIS_CERT_FILE,
        ssl_cert_reqs='required',
        ssl_ca_certs=SecurityConstants.REDIS_CA_FILE,
        ssl_check_hostname=False)
else:
    pool = redis.ConnectionPool(
        connection_class=SSLConnection,
        host=settings.REDIS_HOST,
        port=settings.REDIS_PORT,
        decode_responses=True,
        db=settings.REDIS_DB,
        password=settings.get_redis_password(),
        ssl_keyfile=SecurityConstants.REDIS_KEY_FILE,
        ssl_certfile=SecurityConstants.REDIS_CERT_FILE,
        ssl_cert_reqs='required',
        ssl_ca_certs=SecurityConstants.REDIS_CA_FILE,
        ssl_check_hostname=False
    )
    redis_session = redis.Redis(connection_pool=pool)
