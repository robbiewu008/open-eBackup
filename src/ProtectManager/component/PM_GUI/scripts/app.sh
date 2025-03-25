#!/bin/bash
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

. /app/gui/init_cluster_role.sh
PROTECT_MANAGER_PATH="/opt/OceanProtect/protectmanager"
PROTECT_MANAGER_LOG_NODE_PATH="/opt/OceanProtect/logs/${NODE_NAME}"
PROTECT_MANAGER_PM_LOG_NODE_PATH="${PROTECT_MANAGER_LOG_NODE_PATH}/protectmanager"
PROTECT_MANAGER_KMC_PATH="${PROTECT_MANAGER_PATH}/kmc"
PROTECT_MANAGER_CERT_PATH="${PROTECT_MANAGER_PATH}/cert"
PROTECT_MANAGER_CERT_CA_PATH="${PROTECT_MANAGER_CERT_PATH}/CA"
PROTECT_MANAGER_CERT_CA_CERTS_PATH="${PROTECT_MANAGER_CERT_CA_PATH}/certs"
PROTECT_MANAGER_CERT_INTERNAL_PATH="${PROTECT_MANAGER_CERT_PATH}/internal"
PROTECT_MANAGER_CERT_INTERNAL_OpenAPI_PATH="${PROTECT_MANAGER_CERT_INTERNAL_PATH}/OpenAPI"
PROTECT_MANAGER_WHITE_BOX_PATH="/opt/ProtectManager/whitebox"
PROTECT_MANAGER_DEPLOY_PATH="/opt/ProtectManager/deploy-gui"
PROTECT_MANAGER_I18N_PATH="/opt/ProtectManager/i18n"
PROTECT_MANAGER_I18N_ZH_CODE_PATH="${PROTECT_MANAGER_I18N_PATH}/zh-cn/error-code"
PROTECT_MANAGER_I18N_EN_CODE_PATH="${PROTECT_MANAGER_I18N_PATH}/en-us/error-code"
PROTECT_MANAGER_I18N_ZH_ALARM_PATH="${PROTECT_MANAGER_I18N_PATH}/zh-cn/alarm"
PROTECT_MANAGER_I18N_EN_ALARM_PATH="${PROTECT_MANAGER_I18N_PATH}/en-us/alarm"
PROTECT_MANAGER_WHITE_BOX_RESOURCES_PATH="${PROTECT_MANAGER_WHITE_BOX_PATH}/resources"
APP_CONF_WCC="/app/gui/conf/wcc"
GUI_BASE_PATH="/app/gui/"
ROOT_SCRIPT="/script"

APP_JAR="${GUI_BASE_PATH}app.jar"
FRONT_END_PATH="${GUI_BASE_PATH}frontend"

if [ -n "$SERVICE_MODE" ] && [ "$SERVICE_MODE" == "dev" ]
then
  jvm_param=" -Xdebug -Xrunjdwp:transport=dt_socket,address=5005,server=y,suspend=n "
else
  jvm_param=""
fi


if [[ ! -d "${PROTECT_MANAGER_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_PATH}"
  chmod 750 "${PROTECT_MANAGER_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_LOG_NODE_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_LOG_NODE_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_PM_LOG_NODE_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_PM_LOG_NODE_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_WHITE_BOX_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_WHITE_BOX_PATH}"
  chmod 770 "${PROTECT_MANAGER_WHITE_BOX_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_WHITE_BOX_RESOURCES_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_WHITE_BOX_RESOURCES_PATH}"
  chmod 770 "${PROTECT_MANAGER_WHITE_BOX_RESOURCES_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_DEPLOY_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_DEPLOY_PATH}"
  chmod 750 "${PROTECT_MANAGER_DEPLOY_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_I18N_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_I18N_PATH}"
  chmod 770 "${PROTECT_MANAGER_I18N_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_I18N_ZH_CODE_PATH}" ]]; then
  sudo ${ROOT_SCRIPT}/change_permission.sh change_owner_nobody_nobody  "${PROTECT_MANAGER_I18N_PATH}"
  mkdir -p "${PROTECT_MANAGER_I18N_ZH_CODE_PATH}"
  chmod 770 "${PROTECT_MANAGER_I18N_ZH_CODE_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_I18N_EN_CODE_PATH}" ]]; then
  sudo ${ROOT_SCRIPT}/change_permission.sh change_owner_nobody_nobody  "${PROTECT_MANAGER_I18N_PATH}"
  mkdir -p "${PROTECT_MANAGER_I18N_EN_CODE_PATH}"
  chmod 770 "${PROTECT_MANAGER_I18N_EN_CODE_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_I18N_ZH_ALARM_PATH}" ]]; then
  mkdir -p "${PROTECT_MANAGER_I18N_ZH_ALARM_PATH}"
  chmod 770 "${PROTECT_MANAGER_I18N_ZH_ALARM_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_I18N_EN_ALARM_PATH}" ]]; then
  mkdir -p "${PROTECT_MANAGER_I18N_EN_ALARM_PATH}"
  chmod 770 "${PROTECT_MANAGER_I18N_EN_ALARM_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_I18N_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_I18N_PATH}"
  chmod 750 "${PROTECT_MANAGER_I18N_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_I18N_ZH_CODE_PATH}" ]]; then
  mkdir -p "${PROTECT_MANAGER_I18N_ZH_CODE_PATH}"
  chmod 750 "${PROTECT_MANAGER_I18N_ZH_CODE_PATH}"
fi


if [[ ! -d "${PROTECT_MANAGER_I18N_EN_CODE_PATH}" ]]; then
  mkdir -p "${PROTECT_MANAGER_I18N_EN_CODE_PATH}"
  chmod 750 "${PROTECT_MANAGER_I18N_EN_CODE_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_I18N_ZH_ALARM_PATH}" ]]; then
  mkdir -p "${PROTECT_MANAGER_I18N_ZH_ALARM_PATH}"
  chmod 750 "${PROTECT_MANAGER_I18N_ZH_ALARM_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_I18N_EN_ALARM_PATH}" ]]; then
  mkdir -p "${PROTECT_MANAGER_I18N_EN_ALARM_PATH}"
  chmod 750 "${PROTECT_MANAGER_I18N_EN_ALARM_PATH}"
fi

sudo ${ROOT_SCRIPT}/mount_oper.sh mount_bind "${PROTECT_MANAGER_PM_LOG_NODE_PATH}" "${GUI_BASE_PATH}logs"

sudo ${ROOT_SCRIPT}/change_permission.sh change_owner_nobody_nobody  "/opt/OceanStor-100P/"

find "/opt/OceanStor-100P/" -type d | xargs chmod 700
find "/opt/OceanStor-100P/" -type f | xargs chmod 600

find "${PROTECT_MANAGER_PM_LOG_NODE_PATH}/PM_GUI" -type d | xargs chmod 750
find "${PROTECT_MANAGER_PM_LOG_NODE_PATH}/PM_GUI" -type f | xargs chmod 640

sudo ${ROOT_SCRIPT}/change_permission.sh chmod_group_write /tmp
if [[ -f "/tmp/app.jar" ]]; then
  rm -rf "/tmp/app.jar"
fi

if [[ -d "/tmp/BOOT-INF" ]]; then
  rm -rf "/tmp/BOOT-IN"
fi

FINAL_I18N_PATH="${FRONT_END_PATH}/console/assets/i18n"
sudo ${ROOT_SCRIPT}/change_permission.sh change_owner_nobody_nobody  "${PROTECT_MANAGER_I18N_PATH}"
sudo python3 "${ROOT_SCRIPT}"/xml2json.py "zh" > "${PROTECT_MANAGER_I18N_ZH_CODE_PATH}"/dorado_v6.json
sudo ${ROOT_SCRIPT}/mount_oper.sh mount_bind "${PROTECT_MANAGER_I18N_ZH_CODE_PATH}"/dorado_v6.json "${FINAL_I18N_PATH}/zh-cn/error-code/dorado_v6.json"

sudo python3 "${ROOT_SCRIPT}"/xml2json.py "en" > "${PROTECT_MANAGER_I18N_EN_CODE_PATH}"/dorado_v6.json
sudo ${ROOT_SCRIPT}/mount_oper.sh mount_bind "${PROTECT_MANAGER_I18N_EN_CODE_PATH}"/dorado_v6.json "${FINAL_I18N_PATH}/en-us/error-code/dorado_v6.json"

sudo ${ROOT_SCRIPT}/change_permission.sh change_owner_nobody_nobody  "${FRONT_END_PATH}/console/src/assets/i18n"
sudo ${ROOT_SCRIPT}/change_permission.sh change_owner_nobody_nobody  "${FRONT_END_PATH}/whitebox"
sudo ${ROOT_SCRIPT}/change_permission.sh chmod "${FRONT_END_PATH}/console/src/assets/i18n" 700

cp "/app/app.jar" "/tmp"
cd "/tmp"
sudo ${ROOT_SCRIPT}/change_permission.sh chmod /tmp/app.jar 700

# set enviroment deploy_type a8000 or cloudbackup
sudo ${ROOT_SCRIPT}/mount_oper.sh mount_bind "${PROTECT_MANAGER_DEPLOY_PATH}" "/app/gui/frontend/console/assets/deploy"
deploy_json_dir="${FRONT_END_PATH}/console/assets/deploy/"
deploy_json="deploy.json"
deploy_json_path="$deploy_json_dir""$deploy_json"
echo -e "{ \n \"deploy_type\": \"$DEPLOY_TYPE\" \n }" > "$deploy_json_path"

cd "${GUI_BASE_PATH}"

chmod 775 /whitebox

# 避免重启后cert权限被修改
if [[ ! -d "${PROTECT_MANAGER_CERT_CA_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CERT_CA_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CERT_CA_CERTS_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CERT_CA_CERTS_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CERT_INTERNAL_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CERT_INTERNAL_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CERT_INTERNAL_OpenAPI_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CERT_INTERNAL_OpenAPI_PATH}"
fi

chmod 750 "${PROTECT_MANAGER_PATH}"
chmod 750 "${PROTECT_MANAGER_KMC_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_CA_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_CA_CERTS_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_INTERNAL_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_INTERNAL_OpenAPI_PATH}"
chmod 750 "${PROTECT_MANAGER_DEPLOY_PATH}"
chmod 770 "${PROTECT_MANAGER_WHITE_BOX_PATH}"
chmod 770 "${PROTECT_MANAGER_WHITE_BOX_RESOURCES_PATH}"

### 方法简要说明：
### 1. 是先查找一个字符串：带双引号的key。如果没找到，则直接返回defaultValue。
### 2. 查找最近的冒号，找到后认为值的部分开始了，直到在层数上等于0时找到这3个字符：,}]。
### 3. 如果有多个同名key，则依次全部打印（不论层级，只按出现顺序）
###
### 3 params: json, key, defaultValue
function getJsonValuesByAwk() {
    awk -v json="$1" -v key="$2" -v defaultValue="$3" 'BEGIN{
        foundKeyCount = 0
        while (length(json) > 0) {
            # pos = index(json, "\""key"\""); ## 这行更快一些，但是如果有value是字符串，且刚好与要查找的key相同，会被误认为是key而导致值获取错误
            pos = match(json, "\""key"\"[ \\t]*?:[ \\t]*");
            if (pos == 0) {if (foundKeyCount == 0) {print defaultValue;} exit 0;}

            ++foundKeyCount;
            start = 0; stop = 0; layer = 0;
            for (i = pos + length(key) + 1; i <= length(json); ++i) {
                lastChar = substr(json, i - 1, 1)
                currChar = substr(json, i, 1)

                if (start <= 0) {
                    if (lastChar == ":") {
                        start = currChar == " " ? i + 1: i;
                        if (currChar == "{" || currChar == "[") {
                            layer = 1;
                        }
                    }
                } else {
                    if (currChar == "{" || currChar == "[") {
                        ++layer;
                    }
                    if (currChar == "}" || currChar == "]") {
                        --layer;
                    }
                    if ((currChar == "," || currChar == "}" || currChar == "]") && layer <= 0) {
                        stop = currChar == "," ? i : i + 1 + layer;
                        break;
                    }
                }
            }

            if (start <= 0 || stop <= 0 || start > length(json) || stop > length(json) || start >= stop) {
                if (foundKeyCount == 0) {print defaultValue;} exit 0;
            } else {
                print substr(json, start, stop - start);
            }

            json = substr(json, stop + 1, length(json) - stop)
        }
    }'
}

if [[ -f "${PROTECT_MANAGER_PATH}/timezone" ]]; then
  rm -rf "${PROTECT_MANAGER_PATH}/timezone"
  cp /etc/timezone ${PROTECT_MANAGER_PATH}
fi

isHas=`ifconfig |grep "cbri1601"`
if [ "${DEPLOY_TYPE}" = "d5" ]; then
    # 安全一体机d5场景使用eth1网桥
    ip=${POD_IP}
    if [[ -f "${PROTECT_MANAGER_PATH}/timezone" ]]; then
          rm -rf "${PROTECT_MANAGER_PATH}/timezone"
    fi
    if [[ -f "/etc/timezone" ]]; then
          rm -rf "/etc/timezone"
    fi
elif [[ ! -z "${isHas}" ]]; then
    ip=`ifconfig cbri1601 | grep "inet " | awk '{print $2}'` &> /dev/null
    timezoneJson=`sudo ${ROOT_SCRIPT}/curl_dorado_timezone.sh`
    timezones=`getJsonValuesByAwk "${timezoneJson}" "CMO_SYS_TIME_ZONE_NAME" "Asia/Shanghai"`
    echo "local timezone is $timezoneJson"
    echo "${timezones}" | tr -d '"' > "${PROTECT_MANAGER_PATH}/timezone"
    sudo ${ROOT_SCRIPT}/mount_oper.sh mount_bind "${PROTECT_MANAGER_PATH}/timezone" "/etc/timezone"
else
    ip="0.0.0.0"
    echo 'Asia/Shanghai' > "${PROTECT_MANAGER_PATH}/timezone"
    sudo ${ROOT_SCRIPT}/mount_oper.sh mount_bind "${PROTECT_MANAGER_PATH}/timezone" "/etc/timezone"
fi


function oem() {
    new_oem_file_sha256sum_cmd_res=`sudo /usr/bin/sha256sum /whitebox/oem_package.tgz`
    # 没有新的白牌包导入
    if [ $? -ne 0 ]; then
      echo "/whitebox/oem_package.tgz not exist."
      return
    fi
    new_oem_file_sha256sum=`echo ${new_oem_file_sha256sum_cmd_res} |  awk '{print $1}'`
    old_oem_file_sha256sum_cmd_res=`/usr/bin/sha256sum ${PROTECT_MANAGER_WHITE_BOX_PATH}/oem_package.tgz`
    if [ $? -eq 0 ]; then
      old_oem_file_sha256sum=`echo ${old_oem_file_sha256sum_cmd_res} |  awk '{print $1}'`
      # 新的白牌资源包和老的白牌资源包一样
      if [ "${old_oem_file_sha256sum}" == "${new_oem_file_sha256sum}" ]; then
        echo "new oem_package is same to old oem_package."
        return
      fi
    fi
    sudo /usr/bin/mv -f /whitebox/oem_package.tgz ${PROTECT_MANAGER_WHITE_BOX_PATH}
    sudo ${ROOT_SCRIPT}/change_permission.sh change_owner_nobody_nobody  "${PROTECT_MANAGER_WHITE_BOX_PATH}/oem_package.tgz"
    MAX_SIZE=$[10*1024*1024]
    if [ `zcat ${PROTECT_MANAGER_WHITE_BOX_PATH}/oem_package.tgz | wc -c` -gt ${MAX_SIZE} ];then
       echo "oem_package size should less 10M."
       return
    fi
    tar -xzvf "${PROTECT_MANAGER_WHITE_BOX_PATH}/oem_package.tgz" -C "${PROTECT_MANAGER_WHITE_BOX_RESOURCES_PATH}"
}

function init_gui_common() {
    while true; do
      sleep 1
      # 获取集群角色
      getClusterRole

      if [[ ${CLUSTER_ROLE} == ${MEMBER_ROLE} ]]; then
        # 成员节点空跑gui:
        # 1.杀死原来的java进程
        gui_pid=$(ps -efww | grep 'java' | grep "${APP_JAR}" | awk '{print $2}')
        if [[ ${gui_pid} ]]; then
           kill -9 ${gui_pid}
        fi
        # 2.无空跑任务时进行空跑
        tail_pid=$(ps -efww | grep 'tail -f' | grep '/dev/null'| awk '{print $2}')
        echo "empty task(${tail_pid}) is running."
        if [[ -z ${tail_pid} ]]; then
          echo "start run empty task."
          tail -f /dev/null &
        fi
      else
        # 主、从节点正常运行gui:
        # 1.杀死原来的空跑进程，
        tail_pid=$(ps -efww | grep 'tail -f' | grep '/dev/null'| awk '{print $2}')
        if [[ ${tail_pid} ]]; then
           echo "kill empty task(${tail_pid})."
           kill -9 ${tail_pid}
        fi
        # 2.运行gui进程jar
        process_number=$(ps -efww | grep 'java' | grep "${APP_JAR}" | grep -v 'grep' | wc -l)
          if [[ ${process_number} == 0 ]]; then
              "${JAVA_HOME}"/bin/java -Xms1500M -Xmx1500M -XX:+HeapDumpOnOutOfMemoryError -XX:HeapDumpPath=/opt/ProtectManager/dump/GUI/heapdump.hprof -Djava.awt.headless=true -Dserver.address=$ip -jar ${APP_JAR} &
          fi
      fi
    done
}

sudo ${ROOT_SCRIPT}/mount_oper.sh mount_bind "${PROTECT_MANAGER_WHITE_BOX_PATH}" "/app/gui/frontend/console/assets/whitebox"
sudo ${ROOT_SCRIPT}/mount_oper.sh mount_bind "${PROTECT_MANAGER_KMC_PATH}" "${APP_CONF_WCC}"

# 处理白牌资源包
oem

if [[ "x$DEPLOY_TYPE" == "xd3" || "x$DEPLOY_TYPE" == "xcloudbackup" ]]; then
  while true; do
    sleep 1
     process_number=$(ps -efww | grep 'java' | grep "${APP_JAR}" | grep -v 'grep' | wc -l)
       if [[ ${process_number} == 0 ]]; then
         "${JAVA_HOME}"/bin/java -Xms1100M -Xmx1100M -XX:+HeapDumpOnOutOfMemoryError -XX:HeapDumpPath=/opt/ProtectManager/dump/GUI/heapdump.hprof -Djava.awt.headless=true -Dserver.address=$ip -jar ${APP_JAR}
       fi
  done
else
  init_gui_common
fi