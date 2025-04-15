#!/bin/bash
#########################################
# Copyright (c) 2012-2015 Huawei .
# All rights reserved.
#
# Please send feedback to http://www.huawei.com
#
# Function 替换开源组件kafka被动依赖jar包
#
# 安全jar包 来源公司通用仓（同步到CMC仓）
#
########################################

CURRENT_PATH=$(
  cd $(dirname $0)
  pwd
)
source $CURRENT_PATH/../../commParam.sh

LCRP_XML_PATH=${CURRENT_PATH}/../../../conf/
PACKAGE_PATH="${CURRENT_PATH}/../../../../../../open-source-obligation/Infrastructure_OM/infrastructure"

KAFKA_PATH=${PACKAGE_PATH}/kafka-${kafka_version}
JAR_PATH=${CURRENT_PATH}/package

echo LCRP_HOME=${LCRP_HOME}
cp -rf ${LCRP_XML_PATH}/Setting.xml ${LCRP_HOME}/conf/

function down_package_from_cmc() {
  # 利用lcrp命令从cmc下载库
  cd ${CURRENT_PATH}
  artget pull -os public_dependency_cmc_kafka.xml -ap ${JAR_PATH} -user ${opensource_user} -pwd ${opensource_pwd} -at opensource
  if [ $? -ne 0 ]; then
    echo "[kafka]Download artifact to cmc error"
    exit 1
  fi
}

function upgrade_log4j2_for_kafka() {
  if [ ! -d "${KAFKA_PATH}" ]; then
    echo "kafka package does not exist."
    exit 1
  fi
  cd ${JAR_PATH}

  # 更新 log4j2 配置
  if [ -d "${CURRENT_PATH}/bin" -a -d "${CURRENT_PATH}/config" ]; then
    /usr/bin/cp -Raf ${CURRENT_PATH}/bin/* ${KAFKA_PATH}/bin
    /usr/bin/cp -Raf ${CURRENT_PATH}/config/* ${KAFKA_PATH}/config
  fi
}

function replace_kafka_sucurity_jars() {
  if [ ! -d "${KAFKA_PATH}" ]; then
    echo "kafka package does not exist."
    exit 1
  fi
  cd ${JAR_PATH}

  # jackson
  rm -f ${KAFKA_PATH}/libs/jackson-*.jar
  cp -af jackson-annotations-2.13.5.jar ${KAFKA_PATH}/libs
  cp -af jackson-core-2.13.5.jar ${KAFKA_PATH}/libs
  cp -af jackson-databind-2.13.5.jar ${KAFKA_PATH}/libs
  cp -af jackson-dataformat-csv-2.13.5.jar ${KAFKA_PATH}/libs
  cp -af jackson-datatype-jdk8-2.13.5.jar ${KAFKA_PATH}/libs
  cp -af jackson-jaxrs-base-2.13.5.jar ${KAFKA_PATH}/libs
  cp -af jackson-jaxrs-json-provider-2.13.5.jar ${KAFKA_PATH}/libs
  cp -af jackson-module-jaxb-annotations-2.13.5.jar ${KAFKA_PATH}/libs
  cp -af jackson-module-scala_2.12-2.13.5.jar ${KAFKA_PATH}/libs

  # netty
  rm -f ${KAFKA_PATH}/libs/netty-*.jar
  cp -af netty-buffer-4.1.86.Final.jar ${KAFKA_PATH}/libs
  cp -af netty-codec-4.1.86.Final.jar ${KAFKA_PATH}/libs
  cp -af netty-common-4.1.86.Final.jar ${KAFKA_PATH}/libs
  cp -af netty-handler-4.1.86.Final.jar ${KAFKA_PATH}/libs
  cp -af netty-resolver-4.1.86.Final.jar ${KAFKA_PATH}/libs
  cp -af netty-transport-4.1.86.Final.jar ${KAFKA_PATH}/libs
  cp -af netty-transport-native-epoll-4.1.86.Final.jar ${KAFKA_PATH}/libs
  cp -af netty-transport-native-unix-common-4.1.86.Final.jar ${KAFKA_PATH}/libs

  # rocksdbjni
  rm -f ${KAFKA_PATH}/libs/rocksdbjni-*.jar
  cp -af rocksdbjni-7.1.2.jar ${KAFKA_PATH}/libs

  # zookeeper
  rm -f ${KAFKA_PATH}/libs/zookeeper-*.jar
  cp -af zookeeper-3.8.1.jar ${KAFKA_PATH}/libs
  cp -af zookeeper-jute-3.8.1.jar ${KAFKA_PATH}/libs

  # simple logging facade for java
  rm -f ${KAFKA_PATH}/libs/slf4j-api-*.jar
  cp -af slf4j-api-1.7.36.jar ${KAFKA_PATH}/libs
  rm -f ${KAFKA_PATH}/libs/slf4j-log4j12-*.jar
  cp -af slf4j-log4j12-1.7.36.jar ${KAFKA_PATH}/libs

  # apache maven
  rm -f ${KAFKA_PATH}/libs/maven-artifact-*.jar
  cp -af maven-artifact-3.8.4.jar ${KAFKA_PATH}/libs

  # jetty
  rm -f ${KAFKA_PATH}/libs/jetty-*.jar
  cp -af jetty-*.jar ${KAFKA_PATH}/libs

  upgrade_log4j2_for_kafka
}

function main() {
  down_package_from_cmc
  replace_kafka_sucurity_jars
}

main
