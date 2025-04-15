#!/bin/bash
#########################################
# Copyright (c) 2012-2015 Huawei .
# All rights reserved.
#
# Please send feedback to http://www.huawei.com
#
# Function 替换开源组件ES被动依赖jar包
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

ES_PATH=${PACKAGE_PATH}/elasticsearch-${elasticsearch_version}
JAR_PATH=${CURRENT_PATH}/package

echo LCRP_HOME=${LCRP_HOME}
cp -rf ${LCRP_XML_PATH}/Setting.xml ${LCRP_HOME}/conf/

function down_package_from_cmc() {
  # 利用lcrp命令从cmc下载库
  cd ${CURRENT_PATH}
  artget pull -os public_dependency_cmc_elasticsearch.xml -ap ${JAR_PATH} -user ${opensource_user} -pwd ${opensource_pwd} -at opensource
  if [ $? -ne 0 ]; then
    echo "[elasticsearch]Download artifact to cmc error"
    exit 1
  fi
}

function replace_elasticsearch_sucurity_jars() {
  if [ ! -d "${ES_PATH}" ]; then
    echo "elasticsearch package does not exist."
    exit 1
  fi
  cd ${JAR_PATH}
  # jackson
  rm -f ${ES_PATH}/lib/jackson-*.jar
  cp -af jackson-core-2.13.5.jar ${ES_PATH}/lib
  cp -af jackson-dataformat-cbor-2.12.1.jar ${ES_PATH}/lib
  cp -af jackson-dataformat-smile-2.12.1.jar ${ES_PATH}/lib
  cp -af jackson-dataformat-yaml-2.12.1.jar ${ES_PATH}/lib
  rm -f ${ES_PATH}/modules/ingest-geoip/jackson-*.jar
  cp -af jackson-annotations-2.13.5.jar ${ES_PATH}/modules/ingest-geoip
  cp -af jackson-databind-2.13.5.jar ${ES_PATH}/modules/ingest-geoip
  # httpclient
  rm -f ${ES_PATH}/modules/kibana/httpclient-*.jar
  cp -af httpclient-4.5.13.jar ${ES_PATH}/modules/kibana
  rm -f ${ES_PATH}/modules/reindex/httpclient-*.jar
  cp -af httpclient-4.5.13.jar ${ES_PATH}/modules/reindex

  # netty
  rm -f ${ES_PATH}/modules/transport-netty4/netty-*.jar
  cp -af netty-buffer-4.1.77.Final.jar ${ES_PATH}/modules/transport-netty4/
  cp -af netty-codec-4.1.77.Final.jar ${ES_PATH}/modules/transport-netty4/
  cp -af netty-codec-http-4.1.77.Final.jar ${ES_PATH}/modules/transport-netty4/
  cp -af netty-common-4.1.77.Final.jar ${ES_PATH}/modules/transport-netty4/
  cp -af netty-handler-4.1.77.Final.jar ${ES_PATH}/modules/transport-netty4/
  cp -af netty-resolver-4.1.77.Final.jar ${ES_PATH}/modules/transport-netty4/
  cp -af netty-transport-4.1.77.Final.jar ${ES_PATH}/modules/transport-netty4/

  # snakeyaml
  rm -f ${ES_PATH}/lib/snakeyaml-*.jar
  cp -af snakeyaml-1.32.jar ${ES_PATH}/lib
}

function main() {
  down_package_from_cmc
  replace_elasticsearch_sucurity_jars
}

main
