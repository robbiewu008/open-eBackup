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
import time

from confluent_kafka import KafkaError
from confluent_kafka.admin import AdminClient, NewTopic, KafkaException

from app.common import logger
from app.common.config import settings
from app.common.constants.constant import SecurityConstants, ServiceConstants, MultiClusterKafkaTopicConstants
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException

LOG = logger.get_logger(__name__)
RESOURCE_DELETED_TOPIC = "resource.deleted"


def get_admin_client():
    '''
    :returns: confluent admin client
    '''
    return AdminClient({
        "bootstrap.servers": settings.BOOTSTRAP_SERVER,
        "security.protocol": "SASL_SSL",
        "sasl.mechanism": "PLAIN",
        "sasl.username": settings.KAFKA_USERNAME,
        "sasl.password": settings.get_kafka_password(),
        "ssl.ca.location": SecurityConstants.INTERNAL_CA_FILE,
    })


def list_topics():
    '''
    :returns: list of installed TopicPartitions
    '''
    admin = get_admin_client()
    md = admin.list_topics(timeout=10)
    LOG.debug(f'topics={md.topics}')
    topics = [y for x, y in md.topics.items() if y.error is None]
    LOG.debug(f'Topics (total={len(topics)}): {topics}')
    return topics


def list_topic_names():
    '''
    :returns: list of installed topic names
    '''
    topics = list_topics()
    names = [x.topic for x in topics]
    return names


def create_topics(names, partitions, replication_factor, max_retries):
    LOG.info(f'names={names}, partitions={partitions}, replication_factor={replication_factor}, \
max_retries={max_retries}')
    if max_retries < 1:
        raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                   message="max_retries is invalid")
    topic_names = names
    retry_count = 0
    while True:
        topics_not_created = try_create_topics(
            topic_names, partitions, replication_factor
        )
        if len(topics_not_created) == 0:
            return

        retry_count += 1
        if retry_count >= max_retries:
            raise Exception(
                f'Retrying create topic max_retries={max_retries} exceeded.  topics_not_created={topics_not_created}'
            )

        LOG.info(
            f'{retry_count} retries out of {max_retries} (wait 2sec).  topics_not_created={topics_not_created}'
        )
        topic_names = [i[0] for i in topics_not_created]
        time.sleep(1)


def try_create_topics(names, partitions, replication_factor):
    '''
    Creates topics with partition and replication factor. If topic
    exists it ignore.

    :param: names (List[str]) list of topic names
    :param: partitions (int) topic parition count
    :param: replication_factor (int)  number of copies of a topic in kafka cluster

    :raises: Throws KafkaExceptio when fail to create topic in cluster.
    :return: List of failed topics as tuples (name, kafkaerror). Return an empty list when all topics
             created successfully
    '''
    names = list(set(names))
    LOG.debug(f'names={names}, partitions={partitions}, replication_factor={replication_factor}')

    is_zk_cluster_enabled = settings.get_key_from_config_map(ServiceConstants.MULTI_CLUSTER_CONF,
                                                                ServiceConstants.ZK_CLUSTER)
    if is_zk_cluster_enabled:
        new_topics = [
            NewTopic(name, num_partitions=MultiClusterKafkaTopicConstants.KAFKA_DEFAULT_TOPIC_PARTITIONS,
                     replication_factor=MultiClusterKafkaTopicConstants.KAFKA_DEFAULT_TOPIC_REPLICATION_FACTOR,
                     config={'min.insync.replicas': MultiClusterKafkaTopicConstants.MIN_IN_SYNC_REPLICAS})
            for name in names
        ]
    else:
        new_topics = [
            NewTopic(name, num_partitions=partitions, replication_factor=replication_factor)
            for name in names
        ]

    admin = get_admin_client()

    # asynchrously return dict of <topic, future>
    fs = admin.create_topics(new_topics, request_timeout=len(new_topics) * 2)

    not_created = []
    for topic_name, f in fs.items():
        try:
            f.result()
            LOG.info(f'Topic {topic_name} created')
        except KafkaException as ex:
            kafka_error = ex.args[0]
            if kafka_error.code() == KafkaError.TOPIC_ALREADY_EXISTS:
                LOG.info(f'Topic {topic_name} already exists')
            else:
                not_created.append((topic_name, kafka_error.name()))
                LOG.error(f'Topic {topic_name} not created. error={kafka_error.name()}')

    if len(not_created) > 0:
        LOG.error(f'Fail create {len(not_created)}. not_created={not_created}')

    return not_created
