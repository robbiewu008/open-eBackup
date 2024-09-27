#!/bin/bash

product_name="open-ebackup"
product_version=${MS_IMAGE_TAG}

om_port="8088"
export om_version="1.0"
zookeeper_version="3.8.1"
zookeeper_port="2181"
kafka_version="2.12-3.5.0"
kafka_port="9092"
redis_version="6.2.14"
redis_port="6369"
elasticsearch_version="7.10.2"
elasticsearch_port="9200"
gaussdb_version="505.0.0.SPC1500.B002"
gaussdb_port="6432"
dmc_nginx_port="8089"
CURRENT_PATH=$(cd `dirname $0`; pwd)
INF_DOCKERFILE_PATH=${CURRENT_PATH}/../build/Infrastructure/dockerfiles
OM_DOCKERFILE_PATH=${CURRENT_PATH}/../build/om/dockerfiles
HELM_VALUES_PATH=${CURRENT_PATH}/../build/helm/infrastructure

sed -i "s/gaussdb_version/${gaussdb_version}/g" ${INF_DOCKERFILE_PATH}/${product_name}_gaussdb.dockerfile
sed -i "s/gaussdb_port/${gaussdb_port}/g" ${INF_DOCKERFILE_PATH}/${product_name}_gaussdb.dockerfile
sed -i "s/gaussdb_version/${gaussdb_version}/g" ${INF_DOCKERFILE_PATH}/gaussdb.name
sed -i "s/kafka_version/${kafka_version}/g" ${INF_DOCKERFILE_PATH}/${product_name}_kafka.dockerfile
sed -i "s/kafka_port/${kafka_port}/g" ${INF_DOCKERFILE_PATH}/${product_name}_kafka.dockerfile
sed -i "s/kafka_version/${kafka_version}/g" ${INF_DOCKERFILE_PATH}/kafka.name
sed -i "s/zookeeper_version/${zookeeper_version}/g" ${INF_DOCKERFILE_PATH}/${product_name}_zookeeper.dockerfile
sed -i "s/zookeeper_port/${zookeeper_port}/g" ${INF_DOCKERFILE_PATH}/${product_name}_zookeeper.dockerfile
sed -i "s/zookeeper_version/${zookeeper_version}/g" ${INF_DOCKERFILE_PATH}/zookeeper.name
sed -i "s/redis_version/${redis_version}/g" ${INF_DOCKERFILE_PATH}/${product_name}_redis.dockerfile
sed -i "s/redis_port/${redis_port}/g" ${INF_DOCKERFILE_PATH}/${product_name}_redis.dockerfile
sed -i "s/redis_version/${redis_version}/g" ${INF_DOCKERFILE_PATH}/redis.name
sed -i "s/elasticsearch_version/${elasticsearch_version}/g" ${INF_DOCKERFILE_PATH}/${product_name}_elasticsearch.dockerfile
sed -i "s/elasticsearch_port/${elasticsearch_port}/g" ${INF_DOCKERFILE_PATH}/${product_name}_elasticsearch.dockerfile
sed -i "s/elasticsearch_version/${elasticsearch_version}/g" ${INF_DOCKERFILE_PATH}/elasticsearch.name
sed -i "s/om_version/${product_version}/g" ${OM_DOCKERFILE_PATH}/${product_name}_om.dockerfile
sed -i "s/om_version/${product_version}/g" ${OM_DOCKERFILE_PATH}/om.name
sed -i "s/sftp_version/${product_version}/g" ${INF_DOCKERFILE_PATH}/${product_name}_sftp.dockerfile
sed -i "s/sftp_version/${product_version}/g" ${INF_DOCKERFILE_PATH}/sftp.name

# 修改helm中value值
sed -i "s/gaussdb_port/${gaussdb_port}/g" ${HELM_VALUES_PATH}/values.yaml
sed -i "s/kafka_port/${kafka_port}/g" ${HELM_VALUES_PATH}/values.yaml
sed -i "s/zookeeper_port/${zookeeper_port}/g" ${HELM_VALUES_PATH}/values.yaml
sed -i "s/redis_port/${redis_port}/g" ${HELM_VALUES_PATH}/values.yaml
sed -i "s/elasticsearch_port/${elasticsearch_port}/g" ${HELM_VALUES_PATH}/values.yaml
sed -i "s/om_port/${om_port}/g" ${HELM_VALUES_PATH}/values.yaml
sed -i "s/dmc_nginx_port/${dmc_nginx_port}/g" ${HELM_VALUES_PATH}/values.yaml
