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
sed -i "s/^om_version=.*/om_version=${MS_IMAGE_TAG}/g" $CURRENT_PATH/commParam.sh
source $CURRENT_PATH/commParam.sh
PM_MS_DIR=${CURRENT_PATH}/..
LCRP_XML_PATH=${PM_MS_DIR}/conf
OM_PATH=${binary_path}/Infrastructure_OM/om
REST_PATH=${PM_MS_DIR}/../../REST_API/Infrastructure_OM/om
COMPONENT_TYPE="mspkg"
PRODUCT="dorado"

function down_package_from_cmc()
{
    # 从公司中心库下载python依赖库
    mkdir -p ${PM_MS_DIR}
    mkdir -p ${PM_MS_DIR}/package/3rd
    cd ${PM_MS_DIR}/package/3rd
    mkdir -p requirements
    cd requirements
    pip3 download -r ${PM_MS_DIR}/build/requirements.txt
    # 利用lcrp命令从cmc下载库
    #cd ${LCRP_XML_PATH}
    #lcrp.sh d om_dependency_cmc.xml "agentpath:${PM_MS_DIR}/package/3rd" "deppath:${PM_MS_DIR}/dependency.json"
    #artget下载依赖
    cd ${LCRP_XML_PATH}
    artget pull -d om_dependency_cmc.xml -ap ${PM_MS_DIR}/package/3rd -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ];then
        echo "Download artifact from cmc error"
        exit 1
    fi
    artget pull -os om_dependency_opensource.xml -ap ${PM_MS_DIR}/package/3rd -user ${opensource_user} -pwd ${opensource_pwd} -at opensource
    if [ $? -ne 0 ];then
        echo "Download artifact fron opensource-central error"
        exit 1
    fi
    # 移动sqlalchemy-utils库
    mv ${PM_MS_DIR}/package/3rd/SQLAlchemy_Utils*  ${PM_MS_DIR}/package/3rd/requirements
}

function compile_pkg()
{
    # 编译连接数据库所需的组件
    # 编译动态链接库libpq.so.5.5
    cd ${PM_MS_DIR}/package/3rd
    tar zxf GaussDB-Kernel_*_Server_ARM_Lite.tar.gz
    tar zxf GaussDB-Kernel_*_Euler_64bit_Libpq.tar.gz
    cp lib/libpq.so.5.5 ${PM_MS_DIR}/package/3rd/libpq.so.5.5
    cp lib/libssl.so ${PM_MS_DIR}/package/3rd/
    cp lib/libcrypto.so ${PM_MS_DIR}/package/3rd/
    tar zxf GaussDB-Kernel_*_Euler_64bit_Python.tar.gz
    rm -rf ${PM_MS_DIR}/package/3rd/GaussDB-Kernel*
    rm -rf ${PM_MS_DIR}/package/3rd/GaussDB-Kernel_${gaussdb_version}_Server_ARM_Lite.tar.gz
    # 编译postgresql
    cd ${PM_MS_DIR}/package/3rd
    if [ -d ${PM_MS_DIR}/package/3rd/postgresql_lib ];then
        rm -rf ${PM_MS_DIR}/package/3rd/postgresql_lib
    fi
    mkdir ${PM_MS_DIR}/package/3rd/postgresql_lib
    tar zxf postgresql-${postgresql_version}.tar.gz
    cd postgresql-${postgresql_version}
    # 增加安全编译选项
    sed -i '/CFLAGS = @CFLAGS@/ a\CFLAGS +=-fPIE -fstack-protector-strong -pie' src/Makefile.global.in
    sed -i '/LDFLAGS += @LDFLAGS@/ a\LDFLAGS += -Wl,-z,now -s' src/Makefile.global.in
    # --disable-rpath 禁止rpath
    ./configure --without-readline  --without-zlib --disable-rpath --disable-spinlocks --prefix=${PM_MS_DIR}/package/3rd/postgresql_lib
    make -j8 && make install
    find "${PM_MS_DIR}/package/3rd/postgresql_lib" -type f -name "*.a" | xargs rm -f
    rm -rf ${PM_MS_DIR}/package/3rd/postgresql-${postgresql_version}*
    # 修改psycopg2权限
    cd ${PM_MS_DIR}/package/3rd
    chmod -R 755 psycopg2
    tar -zcf psycopg2.tar.gz psycopg2
    # 修改SQLAlchemy权限
    tar xzf SQLAlchemy-${SQLALCHEMY_VERSION}.tar.gz
    sed -i 's/v,/"PostgreSQL 9.2.1",/'  SQLAlchemy-${SQLALCHEMY_VERSION}/lib/sqlalchemy/dialects/postgresql/base.py
    sed -i '73 a\            for extension in self.extensions:' SQLAlchemy-${SQLALCHEMY_VERSION}/setup.py
    sed -i "74 a\                extension.extra_link_args.append('-Wl,--as-needed')" SQLAlchemy-${SQLALCHEMY_VERSION}/setup.py
    sed -i "75 a\                extension.extra_link_args.append('-Wl,-z,now')" SQLAlchemy-${SQLALCHEMY_VERSION}/setup.py
    sed -i "76 a\                extension.extra_link_args.append('-s')" SQLAlchemy-${SQLALCHEMY_VERSION}/setup.py
    find SQLAlchemy-${SQLALCHEMY_VERSION} -type d | xargs chmod 750
    find SQLAlchemy-${SQLALCHEMY_VERSION} -type f | xargs chmod 640
    find SQLAlchemy-${SQLALCHEMY_VERSION} -type f -name "*.py" | xargs chmod 550
    rm SQLAlchemy-${SQLALCHEMY_VERSION}.tar.gz
    tar -zcf SQLAlchemy-${SQLALCHEMY_VERSION}.tar.gz SQLAlchemy-${SQLALCHEMY_VERSION}
}

function build_pkg()
{
    # 拷贝shell脚本和运行包
    if [ ! -d ${PM_MS_DIR}/package/script ];then
        mkdir ${PM_MS_DIR}/package/script
    fi
    
    cp ${CURRENT_PATH}/../scripts/check_health.sh ${PM_MS_DIR}/package
    cp ${CURRENT_PATH}/../scripts/run.sh ${PM_MS_DIR}/package
    cp ${CURRENT_PATH}/../scripts/delete_infrastructure_conf.sh ${PM_MS_DIR}/package/script
    cp ${CURRENT_PATH}/../scripts/export_log.sh ${PM_MS_DIR}/package/script
    cp ${CURRENT_PATH}/../scripts/service_log_process.sh ${PM_MS_DIR}/package/script
    cp ${CURRENT_PATH}/../scripts/common.sh ${PM_MS_DIR}/package/script
    cp ${CURRENT_PATH}/../scripts/change_permission.sh ${PM_MS_DIR}/package/script
    cp ${CURRENT_PATH}/../scripts/infra_init_ctnr_gaussdb.sh ${PM_MS_DIR}/package/script
    # 修改权限
    chmod 550 ${PM_MS_DIR}/package/run.sh
    chmod 550 ${PM_MS_DIR}/package/check_health.sh
    chmod -R 550 ${PM_MS_DIR}/package/script

    if [ ! -d ${PM_MS_DIR}/package/upgrade ];then
        mkdir ${PM_MS_DIR}/package/upgrade
    fi
    cp ${CURRENT_PATH}/../scripts/dpa_upgrade_util.py ${PM_MS_DIR}/package/upgrade
    cp ${CURRENT_PATH}/../scripts/post_rollback_job.py ${PM_MS_DIR}/package/upgrade
    cp ${CURRENT_PATH}/../scripts/pre_rollback_job.py ${PM_MS_DIR}/package/upgrade
    cp ${CURRENT_PATH}/../scripts/post_upgrade_job.py ${PM_MS_DIR}/package/upgrade
    cp ${CURRENT_PATH}/../scripts/pre_upgrade_job.py ${PM_MS_DIR}/package/upgrade
    cp ${CURRENT_PATH}/../scripts/dm_client.py ${PM_MS_DIR}/package/upgrade
    cp ${CURRENT_PATH}/../scripts/pre_create_kmc_configmap_job.py ${PM_MS_DIR}/package/upgrade
    cp ${CURRENT_PATH}/../scripts/pre_upgrade_job_remote.py ${PM_MS_DIR}/package/upgrade
    cp ${CURRENT_PATH}/../scripts/pre_create_ha_secret_job.py ${PM_MS_DIR}/package/upgrade
    cp ${CURRENT_PATH}/../scripts/KmcConfig.yaml ${PM_MS_DIR}/package/upgrade
    cp ${CURRENT_PATH}/../scripts/secret-pm-ha.yaml ${PM_MS_DIR}/package/upgrade
    cp ${CURRENT_PATH}/../scripts/pre_upgrade_checker_job.py ${PM_MS_DIR}/package/upgrade
    cp ${CURRENT_PATH}/../scripts/upgrade_precondition_job.py ${PM_MS_DIR}/package/upgrade

    # 拷贝infra证书脚本和配置文件
    if [ ! -d ${PM_MS_DIR}/package/cert/script ];then
        mkdir -p ${PM_MS_DIR}/package/cert/script
    fi
    if [ ! -d ${PM_MS_DIR}/package/cert/conf ];then
        mkdir -p ${PM_MS_DIR}/package/cert/conf
    fi
    cp ${CURRENT_PATH}/../scripts/generate.sh ${PM_MS_DIR}/package/cert/script
    cp ${CURRENT_PATH}/../scripts/generate_infra_cert.py ${PM_MS_DIR}/package/cert/script
    cp ${CURRENT_PATH}/../scripts/init_infra_cert.sh ${PM_MS_DIR}/package/cert/script
    cp ${CURRENT_PATH}/../conf/template.openssl.cnf ${PM_MS_DIR}/package/cert/conf
    cp ${CURRENT_PATH}/../conf/template_gen.openssl.cnf ${PM_MS_DIR}/package/cert/conf

    cp ${CURRENT_PATH}/../scripts/run_modify_db.py ${PM_MS_DIR}/package/3rd
    cp -r ${PM_MS_DIR}/src ${PM_MS_DIR}/package/
    cd ${PM_MS_DIR}

    find "${PM_MS_DIR}/package/3rd" -type d | xargs chmod 750
    find "${PM_MS_DIR}/package/3rd" -type f | xargs chmod 640
    OM_3RD_PATH="${PM_MS_DIR}/package/3rd"
    # postgresql_lib/bin
    [ -d "${OM_3RD_PATH}/postgresql_lib/bin" ] && chmod -R 550 ${OM_3RD_PATH}/postgresql_lib/bin
    # postgresql_lib/include
    [ -d "${OM_3RD_PATH}/postgresql_lib/include" ] && chmod -R 550 ${OM_3RD_PATH}/postgresql_lib/include
    # postgresql_lib/lib
    [ -d "${OM_3RD_PATH}/postgresql_lib/lib" ] && chmod -R 550 ${OM_3RD_PATH}/postgresql_lib/lib
    # requirements
    [ -d "${OM_3RD_PATH}/requirements" ] && chmod -R 550 ${OM_3RD_PATH}/requirements

    find "${PM_MS_DIR}/package/src" -type d | xargs chmod 750
    find "${PM_MS_DIR}/package/src" -type f | xargs chmod 640
    find "${PM_MS_DIR}/package/src" -type f -name "*.py" | xargs chmod 550

    chmod 750 package
    tar -czf ${om_name}-${om_version}.tar.gz package/
    mkdir -p ${OM_PATH}
    mkdir -p ${OM_PATH}/pkg
	  cp -f ${PM_MS_DIR}/${om_name}-${om_version}.tar.gz ${OM_PATH}/pkg/
	  cp -rf ${PM_MS_DIR}/package ${REST_PATH}/
	  if [ $? -ne 0 ]; then
    echo "cp om tar.gz to WORKSPACE failed"
    exit 1
    fi

    echo "cp om tar.gz success"
}

function main()
{
    if [ ${MS_IMAGE_TAG} == " " ]; then
        echo "MS_IMAGE_TAG is NULL"
        exit 1
    fi
    down_package_from_cmc
    compile_pkg
    build_pkg
}

main