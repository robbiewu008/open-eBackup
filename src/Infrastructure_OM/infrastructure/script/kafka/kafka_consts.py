#!/usr/bin/env python
# _*_ coding:utf-8 _*_

# This file is consts of kafka container.
# It should not be merge with other container's consts file
# even if there have some same consts.
KAFKA_USER_PATH = '/etc/common-secret/kafka.username'
KAFKA_PWD_PATH = '/etc/common-secret/kafka.password'
KAFKA_JAAS_PATH = '/opt/third_data/kafka/config/kafka_server_jaas.conf'
KAFKA_STATE_TXT_PATH = '/opt/third_data/kafka/kafka_state_flag.txt'
PARTITION_STATE_CHANGE_FAILED_MSG = 'failed to change state for partition'
CLIENT_SHUTDOWN_MSG = 'java.io.IOException: Client was shutdown before response was read'
INVALID_REPLICATION_FACTOR_MSG = 'Kafka Exception: org.apache.kafka.common.errors.InvalidReplicationFactorException.'
INVALID_REPLICATION_FACTOR_VAL_MSG = "value='.apache.kafka.common.errors.InvalidReplicationFactorException:"
