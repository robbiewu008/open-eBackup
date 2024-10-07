#!/bin/bash
########################################
#  This file is part of the open-eBackup project.
# Copyright (c) 2024 Huawei Technologies Co.,Ltd.
#
# open-eBackup is licensed under MPL v2.
# You can use this software according to the terms and conditions of the MPL v2.
# You may obtain a copy of MPL v2 at:
#
#          https://www.mozilla.org/en-US/MPL/2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the MPL v2 for more details.
########################################
set -x
CURRENT_PATH=$(cd `dirname $0`; pwd)
source $CURRENT_PATH/commParam.sh
LCRP_XML_PATH=${CURRENT_PATH}/../conf/
PACKAGE_PATH=${CURRENT_PATH}/../../package
COMPILE_PATH=${PACKAGE_PATH}/compileLib
BIN_PATH=${CURRENT_PATH}/../../../open-source-obligation/Infrastructure_OM/infrastructure
if [ ! -d ${COMPILE_PATH} ];then
  mkdir -p ${COMPILE_PATH}
fi
REDIS_PATH=${PACKAGE_PATH}/redis
SFTP_PATH=${PACKAGE_PATH}/sftp
ZK_LOG4J2_PATH=${CURRENT_PATH}/upgrade_opensrc/zookeeper/zk_log4j2
shopt -s extglob
COMPONENT_TYPE="mspkg"
PRODUCT="dorado"

if [ -z "${componentVersion}" ]; then
    componentVersion="1.1.0"
fi
echo "Component Version:${componentVersion}"

function down_package_from_cmc()
{
    #artget下载依赖
    cd ${LCRP_XML_PATH}
    artget pull -d public_dependency_cmc.xml -p "{'componentVersion':'${componentVersion}'}" -ap ${PACKAGE_PATH} -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ];then
        echo "Download artifact from cmc error"
        exit 1
    fi

    artget pull -os public_dependency_opensource.xml -ap ${PACKAGE_PATH} -user ${opensource_user} -pwd ${opensource_pwd} -at opensource
    if [ $? -ne 0 ];then
        echo "Download artifact from cmc error"
        exit 1
    fi

    artget pull -d opensrouce_from_centralized.xml -p "{'CODE_BRANCH':'${CODE_BRANCH}'}" -ap ${PACKAGE_PATH} -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ];then
        echo "Download opensource pkg from centralized cmc error."
        exit 1
    fi
}

function compile_kafka_package()
{
    msg_queue_version="${MS_IMAGE_TAG}"
    # kafka
    cd ${PACKAGE_PATH}

    if [ -d kafka-scripts ];then
        rm -rf kafka-scripts
    fi
    mkdir kafka-scripts
    cp -r "${CURRENT_PATH}"/kafka/* kafka-scripts/
    cp -r "${CURRENT_PATH}"/common kafka-scripts/
    cp -r "${CURRENT_PATH}"/kmc kafka-scripts/
    cp "${CURRENT_PATH}"/mount_oper.sh kafka-scripts/
    sed -i "s/JDK_VERSION/${jdk_version}/g" kafka-scripts/install_kafka.sh
    sed -i "s/VERSION/${kafka_version}/g" kafka-scripts/install_kafka.sh
    sed -i "s/VERSION/${kafka_version}/g" kafka-scripts/update_password.sh
    sed -i "s/zookeeper_port/${zookeeper_port}/g" kafka-scripts/install_kafka.sh
    sed -i "s/kafka_port/${kafka_port}/g" kafka-scripts/install_kafka.sh
    sed -i "s/kafka_port/${kafka_port}/g" kafka-scripts/check_health.sh
    chmod -R 550 kafka-scripts/
    tar -zcvf ${COMPILE_PATH}/kafka-scripts.tar.gz kafka-scripts/*
    cp -rf ${COMPILE_PATH}/kafka-scripts.tar.gz ${BIN_PATH}

    if [ -d kafka-${kafka_version} ];then
        rm -rf kafka-${kafka_version}
    fi
    tar -xzvf kafka_${kafka_version}.tgz
    rm -rf kafka_${kafka_version}/site-docs
    mv kafka_${kafka_version} kafka-${kafka_version}
    # 需要则更新kafka依赖jar包
    if [ -f "$CURRENT_PATH/upgrade_opensrc/kafka/replace_kafka_security_jars.sh" ]; then
        sh $CURRENT_PATH/upgrade_opensrc/kafka/replace_kafka_security_jars.sh
    fi

    if [ ! -d kafka-${kafka_version}/logs ];then
      mkdir kafka-${kafka_version}/logs
    fi
    if [ ! -d kafka-${kafka_version}/data ];then
      mkdir kafka-${kafka_version}/data
      chmod 750 kafka-${kafka_version}/data
    fi
    # 用于微服务认证
    touch kafka-${kafka_version}/config/kafka_server_jaas.conf

    # 权限修改
    # kafka 主目录
    chmod -R 750 kafka-${kafka_version}
    # bin
    chmod -R 550 kafka-${kafka_version}/bin
    # config
    chmod -R 750 kafka-${kafka_version}/config
    find "kafka-${kafka_version}/config/" -type f | xargs chmod 640
    # libs
    chmod -R 550 kafka-${kafka_version}/libs
    tar -zcvf ${COMPILE_PATH}/kafka-${kafka_version}.tar.gz kafka-${kafka_version}/
    cp -rf ${COMPILE_PATH}/kafka-${kafka_version}.tar.gz ${BIN_PATH}
}

function compile_zk_package()
{
    # zookeeper
    cd ${PACKAGE_PATH}
    # zookeeper
    if [ -d zookeeper-${zookeeper_version} ];then
        rm -rf zookeeper-${zookeeper_version}
    fi

    # step1: 解压源码包
    tar -xzvf apache-zookeeper-${zookeeper_version}.tar.gz

    # step2: 更新pom.xml，log4j.properties，zkServer.sh,因为alias cp='cp -i'，需要先删除后写入
    rm -f apache-zookeeper-${zookeeper_version}/pom.xml
    cp ${ZK_LOG4J2_PATH}/pom.xml apache-zookeeper-${zookeeper_version}/pom.xml
    rm -f apache-zookeeper-${zookeeper_version}/zookeeper-server/pom.xml
    cp ${ZK_LOG4J2_PATH}/zookeeper-server/pom.xml apache-zookeeper-${zookeeper_version}/zookeeper-server/pom.xml
    rm -f apache-zookeeper-${zookeeper_version}/bin/zkServer.sh
    cp ${ZK_LOG4J2_PATH}/bin/zkServer.sh apache-zookeeper-${zookeeper_version}/bin/zkServer.sh
    sed -i "s|\"-Dlog4j.configurationFile=\${ZOOCFGDIR}/log4j.properties\"|\"-Dlog4j.configurationFile=\${ZOOCFGDIR}/log4j.properties\"\ \"-Dzookeeper.admin.enableServer=false\"|g" apache-zookeeper-${zookeeper_version}/bin/zkServer.sh
    rm -f apache-zookeeper-${zookeeper_version}/conf/log4j.properties
    cp ${ZK_LOG4J2_PATH}/conf/log4j.properties apache-zookeeper-${zookeeper_version}/conf/log4j.properties

    # step3: 执行编译(marven)，生成apache-zookeeper-${zookeeper_version}-bin.tar.gz
    cd ${PACKAGE_PATH}/apache-zookeeper-${zookeeper_version}
    mvn install -Dmaven.test.skip=true -Dmaven.wagon.http.ssl.insecure=true -Dmaven.wagon.http.ssl.allowall=true

    cd ${PACKAGE_PATH}
    # step4: 后处理，拷贝出源码包，删除解包的源码工程，继续原有处理
    mv apache-zookeeper-${zookeeper_version}/zookeeper-assembly/target/apache-zookeeper-${zookeeper_version}-bin.tar.gz ./
    rm apache-zookeeper-${zookeeper_version} -rf
    tar -xzvf apache-zookeeper-${zookeeper_version}-bin.tar.gz
    rm -rf apache-zookeeper-${zookeeper_version}-bin/docs
    cp ${CURRENT_PATH}/zookeeper/install_zookeeper.sh apache-zookeeper-${zookeeper_version}-bin
    sed -i "s/JDK_VERSION/${jdk_version}/g" apache-zookeeper-${zookeeper_version}-bin/install_zookeeper.sh
    sed -i "s/VERSION/${zookeeper_version}/g" apache-zookeeper-${zookeeper_version}-bin/install_zookeeper.sh
    cp ${CURRENT_PATH}/zookeeper/check_health.sh apache-zookeeper-${zookeeper_version}-bin
    sed -i "s/zookeeper_port/${zookeeper_port}/g" apache-zookeeper-${zookeeper_version}-bin/check_health.sh
    mv apache-zookeeper-${zookeeper_version}-bin zookeeper-${zookeeper_version}
    cp ${CURRENT_PATH}/mount_oper.sh zookeeper-${zookeeper_version}/
    if [ ! -d zookeeper-${zookeeper_version}/logs ];then
      mkdir zookeeper-${zookeeper_version}/logs
    fi

    # 配置文件参数修改
    sed -i "/^zookeeper.root.logger=.*/c\zookeeper.root.logger=INFO,RFAAUDIT" zookeeper-${zookeeper_version}/conf/log4j.properties
    cp zookeeper-${zookeeper_version}/conf/zoo_sample.cfg zookeeper-${zookeeper_version}/conf/zoo.cfg
    # change permission
    chmod -R 750 zookeeper-${zookeeper_version}
    # bin
    [ -d "zookeeper-${zookeeper_version}/bin" ] && chmod -R 550 zookeeper-${zookeeper_version}/bin
    # conf
    [ -d "zookeeper-${zookeeper_version}/conf" ] && chmod -R 750 zookeeper-${zookeeper_version}/conf
    # lib
    [ -d "zookeeper-${zookeeper_version}/lib" ] && chmod -R 550 zookeeper-${zookeeper_version}/lib

    tar -zcvf ${COMPILE_PATH}/zookeeper-${zookeeper_version}.tar.gz zookeeper-${zookeeper_version}/
    cp -rf ${COMPILE_PATH}/zookeeper-${zookeeper_version}.tar.gz ${BIN_PATH}
}

function compile_es_package()
{
    # es
    cd ${PACKAGE_PATH}

    wget -c https://cmc.cloudartifact.szv.dragon.tools.huawei.com/artifactory/customized-generic-oss/elasticsearch-oss/7.10.2_7.10.2-h5/release/elasticsearch-oss-7.10.2-h5-no-jdk-linux-x86_64.tar.gz --no-check-certificate

    tar -xzvf elasticsearch-oss-7.10.2-h5-no-jdk-linux-x86_64.tar.gz
    cp ${CURRENT_PATH}/elasticsearch/install_elasticsearch.sh elasticsearch-${elasticsearch_version}
    cp ${CURRENT_PATH}/elasticsearch/net_plane_ip.py elasticsearch-${elasticsearch_version}

    sed -i "s/JDK_VERSION/${jdk_version}/g" elasticsearch-${elasticsearch_version}/install_elasticsearch.sh
    sed -i "s/VERSION/${elasticsearch_version}/g" elasticsearch-${elasticsearch_version}/install_elasticsearch.sh
    cp ${CURRENT_PATH}/elasticsearch/check_health.sh elasticsearch-${elasticsearch_version}
    cp ${CURRENT_PATH}/elasticsearch/check_elasticsearch_readiness.sh elasticsearch-${elasticsearch_version}
    cp ${CURRENT_PATH}/mount_oper.sh elasticsearch-${elasticsearch_version}
    sed -i "s/elasticsearch_port/${elasticsearch_port}/g" elasticsearch-${elasticsearch_version}/check_health.sh
    sed -i "s/elasticsearch_port/${elasticsearch_port}/g" elasticsearch-${elasticsearch_version}/check_elasticsearch_readiness.sh
    # 需要则更新elasticsearch依赖jar包
    if [ -f "$CURRENT_PATH/upgrade_opensrc/elasticsearch/replace_elasticsearch_security_jars.sh" ]; then
        sh $CURRENT_PATH/upgrade_opensrc/elasticsearch/replace_elasticsearch_security_jars.sh
    fi

    if [ ! -d elasticsearch-${elasticsearch_version}/logs ];then
      mkdir elasticsearch-${elasticsearch_version}/logs
    fi
    if [ ! -d elasticsearch-${elasticsearch_version}/data ];then
      mkdir elasticsearch-${elasticsearch_version}/data
      chmod 750 elasticsearch-${elasticsearch_version}/data
    fi

    # elasticsearch
    chmod -R 750 elasticsearch-${elasticsearch_version}/
    # bin
    chmod -R 550 elasticsearch-${elasticsearch_version}/bin
    # config
    chmod -R 750 elasticsearch-${elasticsearch_version}/config
    find "elasticsearch-${elasticsearch_version}/config/" -type f | xargs chmod 640
    # lib
    chmod -R 550 elasticsearch-${elasticsearch_version}/lib
    # logs
    chmod -R 750 elasticsearch-${elasticsearch_version}/logs

    tar -zcvf ${COMPILE_PATH}/elasticsearch-${elasticsearch_version}.tar.gz elasticsearch-${elasticsearch_version}/
    cp -rf ${COMPILE_PATH}/elasticsearch-${elasticsearch_version}.tar.gz ${BIN_PATH}
}

function compile_redis_package()
{
    # redis
    cd ${PACKAGE_PATH}

    if [ -d redis-scripts ];then
        rm -rf redis-scripts
    fi
    mkdir redis-scripts
    cp -r "${CURRENT_PATH}"/redis/* redis-scripts/
    cp -r "${CURRENT_PATH}"/common redis-scripts/
    cp -r "${CURRENT_PATH}"/kmc redis-scripts/
    cp "${CURRENT_PATH}"/mount_oper.sh redis-scripts/
    sed -i "s/redis_port/${redis_port}/g" redis-scripts/check_health.sh
    sed -i "s/VERSION/${redis_version}/g" redis-scripts/install_redis.sh
    sed -i "s/VERSION/${redis_version}/g" redis-scripts/update_password.sh
    chmod -R 550 redis-scripts/
    tar -zcvf ${COMPILE_PATH}/redis-scripts.tar.gz redis-scripts/
    cp -rf ${COMPILE_PATH}/redis-scripts.tar.gz ${BIN_PATH}

    tar -xzvf redis-${redis_version}.tar.gz
    # 如果redis目录不存在
    if [ -d ${REDIS_PATH} ];then
        rm -rf ${REDIS_PATH}
    fi
    mkdir ${REDIS_PATH}

    #修改Makefile文件吗，增加安全编译选项
    cd redis-${redis_version}

    # redis漏洞修复，patch必须按顺序
    patch_list=("")
    for patch_file in ${patch_list[*]}
    do

        if [ ! -f "${PACKAGE_PATH}/patch/common/${patch_file}" ]; then
            echo "Can't Find ${patch_file}"
            exit 1
        fi
        patch -p1 < ${PACKAGE_PATH}/patch/common/${patch_file}
        if [ $? -ne 0 ]; then
            echo "Make Patch(${patch_file}) Error !"
            exit 1
        fi
    done
    cp -af "${CURRENT_PATH}"/redis/Makefile.global .
    sed -i "56 i\include ..\/Makefile.global" src/Makefile
    sed -i "12 i\include ..\/Makefile.global" deps/Makefile
    sed -i "4 i\include ..\/..\/Makefile.global" deps/linenoise/Makefile
    sed -i "11 i\include ..\/..\/..\/Makefile.global" deps/lua/src/Makefile

    #编译redis依赖的包
    make MALLOC=libc BUILD_TLS=yes && make install
    if [ $? -ne 0 ];then
        echo "compile redis package error"
        exit 1
    fi
    mv src/redis-* ${REDIS_PATH}
    rm -rf ${REDIS_PATH}/redis-trib.rb
    mv redis.conf ${REDIS_PATH}

    cd ${PACKAGE_PATH}
    rm -rf redis-${redis_version}
    mv ${REDIS_PATH} redis-${redis_version}

    find redis-${redis_version}/ -type f -regex '.*\.o' | xargs rm -f

    mkdir redis-${redis_version}/logs

    mkdir redis-${redis_version}/data
    # redis 主目录
    find redis-${redis_version} -type f | xargs chmod 640
    chmod 500 redis-${redis_version}/redis-benchmark
    chmod 500 redis-${redis_version}/redis-check-aof
    chmod 500 redis-${redis_version}/redis-check-rdb
    chmod 500 redis-${redis_version}/redis-sentinel
    chmod 500 redis-${redis_version}/redis-cli
    chmod 500 redis-${redis_version}/redis-server

    tar -zcvf ${COMPILE_PATH}/redis-${redis_version}.tar.gz redis-${redis_version}/
    cp -rf ${COMPILE_PATH}/redis-${redis_version}.tar.gz ${BIN_PATH}
}

function compile_sftp_package()
{
    sftp_version="${MS_IMAGE_TAG}"
    cd "${PACKAGE_PATH}"
    if [ -d "${SFTP_PATH}" ];then
        rm -rf "${SFTP_PATH}"
    fi

    mkdir -p "${SFTP_PATH}/package"
    chmod 750 "${SFTP_PATH}/package"
    mkdir -p "${SFTP_PATH}/package/script"
    mkdir "${SFTP_PATH}/package/requirements"
    cd "${SFTP_PATH}/package/requirements"
    pip3 download -r ${CURRENT_PATH}/sftp/requirements.txt

    tar -zxvf pyrsistent-*.tar.gz
    cd pyrsistent-*
    python3 setup.py sdist bdist_wheel
    mv ./dist/pyrsistent-*-cp39-cp39-linux_aarch64.whl ..
    cd ..

    cd "${PACKAGE_PATH}"
    cp -r "${CURRENT_PATH}/sftp/src" "${SFTP_PATH}/package/"
    cp "${CURRENT_PATH}/sftp/actual_install.sh" "${SFTP_PATH}/package/"
    cp "${CURRENT_PATH}/sftp/check_health.sh" "${SFTP_PATH}/package/script"
    cp "${CURRENT_PATH}/sftp/install_sftp.sh" "${SFTP_PATH}/package/script"
    chmod 550 "${SFTP_PATH}/package/actual_install.sh"
    chmod 550 "${SFTP_PATH}/package/script/check_health.sh"
    chmod 550 "${SFTP_PATH}/package/script/install_sftp.sh"
    find "${SFTP_PATH}/package/src" -type d | xargs chmod 750
    find "${SFTP_PATH}/package/src" -type f | xargs chmod 640
    find "${SFTP_PATH}/package/src" -type f -name "*.py" | xargs chmod 550
    tar -zcvf ${COMPILE_PATH}/sftp-${sftp_version}.tar.gz sftp/
    cp -rf ${COMPILE_PATH}/sftp-${sftp_version}.tar.gz ${BIN_PATH}
}

function compile_package()
{
    # 编译publicLib下的三方开源软件，kafka、zookeeper、redis、es
    compile_kafka_package
    compile_es_package
    compile_zk_package
    compile_redis_package
    compile_sftp_package
}


function main()
{
    down_package_from_cmc
    compile_package
}

main
