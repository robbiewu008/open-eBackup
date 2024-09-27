#!/bin/bash

PROTECT_MANAGER_PATH="/opt/OceanProtect/protectmanager"
PM_PRIVATE_PATH="/opt/ProtectManager"
PROTECT_MANAGER_LOG_NODE_PATH="/opt/OceanProtect/logs/${NODE_NAME}"
PROTECT_MANAGER_PM_LOG_NODE_PATH="${PROTECT_MANAGER_LOG_NODE_PATH}/protectmanager"
PROTECT_MANAGER_CERT_PATH="${PROTECT_MANAGER_PATH}/cert"
PROTECT_MANAGER_SYSBACKUP_PATH="${PROTECT_MANAGER_PATH}/sysbackup"
PROTECT_MANAGER_ALARM_PATH="${PM_PRIVATE_PATH}/alarm"
PROTECT_MANAGER_KMC_PATH="${PROTECT_MANAGER_PATH}/kmc"
PROTECT_MANAGER_AGENT_PATH="${PM_PRIVATE_PATH}/agent"
PROTECT_MANAGER_EXPORT_PATH="/opt/ProtectManager/export"
PROTECT_MANAGER_DUMP_PATH="/opt/ProtectManager/dump/System_Base"
PROTECT_MANAGER_AGENT_LOG_PATH="/opt/ProtectManagerAgentLog"
PROTECT_MANAGER_REPORT_PATH="/opt/ProtectManager/report"
OCEAN_PROTECT_STORAGE_CONFIG_EXPORT_PATH="/opt/OceanProtect/storage_config_export"

# nas新的挂载目录
PM_PRIVATE_PATH_NAS="/opt/ProtectManagerNas"
PROTECT_MANAGER_ALARM_PATH_NAS="${PM_PRIVATE_PATH_NAS}/alarm"
PROTECT_MANAGER_AGENT_PATH_NAS="${PM_PRIVATE_PATH_NAS}/agent"
PROTECT_MANAGER_EXPORT_PATH_NAS="${PM_PRIVATE_PATH_NAS}/export"

PROTECT_MANAGER_CERT_CA_PATH="${PROTECT_MANAGER_CERT_PATH}/CA"
PROTECT_MANAGER_CERT_CA_CERTS_PATH="${PROTECT_MANAGER_CERT_CA_PATH}/certs"
PROTECT_MANAGER_CRL_PATH="${PROTECT_MANAGER_CERT_PATH}/crl"
PROTECT_MANAGER_CERT_INTERNAL_PATH="${PROTECT_MANAGER_CERT_PATH}/internal"
PROTECT_MANAGER_CERT_INTERNAL_OpenAPI_PATH="${PROTECT_MANAGER_CERT_INTERNAL_PATH}/OpenAPI"
PROTECT_MANAGER_CERT_INTERNAL_Agent_PATH="${PROTECT_MANAGER_CERT_INTERNAL_PATH}/ProtectAgent"
PROTECT_MANAGER_CERT_INTERNAL_Agent_Ca_PATH="${PROTECT_MANAGER_CERT_INTERNAL_Agent_PATH}/ca"

PROTECT_MANAGER_I18N_PATH="/opt/ProtectManager/i18n"
PROTECT_MANAGER_I18N_ZH_ALARM_PATH="${PROTECT_MANAGER_I18N_PATH}/zh-cn/alarm"
PROTECT_MANAGER_I18N_EN_ALARM_PATH="${PROTECT_MANAGER_I18N_PATH}/en-us/alarm"

APP_LOG="/app/logs"
APP_CERT="/app/cert"
APP_ALARM="/app/alarm"
APP_AGENT="/app/agent"
APP_CONF_WCC="/app/conf/wcc"
APP_JAR="/app/app.jar"
export LANG="C.utf8"
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

sudo /script/mount_oper.sh mount_bind "${PROTECT_MANAGER_PM_LOG_NODE_PATH}" "${APP_LOG}"

if [[ ! -d "${PROTECT_MANAGER_SYSBACKUP_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_SYSBACKUP_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CERT_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CERT_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_I18N_ZH_ALARM_PATH}" ]]; then
  mkdir -p "${PROTECT_MANAGER_I18N_ZH_ALARM_PATH}"
  chmod 750 "${PROTECT_MANAGER_I18N_ZH_ALARM_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_I18N_EN_ALARM_PATH}" ]]; then
  mkdir -p "${PROTECT_MANAGER_I18N_EN_ALARM_PATH}"
  chmod 750 "${PROTECT_MANAGER_I18N_EN_ALARM_PATH}"
fi

sudo /script/mount_oper.sh mount_bind "${PROTECT_MANAGER_CERT_PATH}" "${APP_CERT}"

if [[ ! -d "${PROTECT_MANAGER_DUMP_PATH}" ]]; then
  mkdir -p "${PROTECT_MANAGER_DUMP_PATH}"
fi

function has_dir_content() {
    if [[ -d ${1} && "`ls -A ${1}`" != "" ]]; then
        echo "true"
    else
        echo "false"
    fi
}

function cp_data_between_two_dir() {
    origin=`has_dir_content ${1}`
    update=`has_dir_content ${2}`
    if [[ ${origin} == "true" && ${update} != "true" ]]; then
        echo "copy dir ${1} to ${2}"
        need_to_copy="${1}*"
        cp -r ${need_to_copy} "${2}"
    elif [[ ${origin} != "true" && ${update} == "true" ]]; then
        echo "copy dir ${2} to ${1}"
        need_to_copy="${2}*"
        cp -r ${need_to_copy} "${1}"
    elif [[ ${origin} == "true" && ${update} == "true" ]]; then
        echo "dir ${1} and ${2} both have content"
    else
        echo "dir ${1} and ${2} both do not have content."
    fi
}

if [[ ! -d "${PROTECT_MANAGER_ALARM_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_ALARM_PATH}"
  chmod 700 "${PROTECT_MANAGER_ALARM_PATH}"
fi
if [[ ! -d "${PROTECT_MANAGER_ALARM_PATH_NAS}" ]]; then
  mkdir "${PROTECT_MANAGER_ALARM_PATH_NAS}"
  chmod 700 "${PROTECT_MANAGER_ALARM_PATH_NAS}"
fi
cp_data_between_two_dir "${PROTECT_MANAGER_ALARM_PATH}" "${PROTECT_MANAGER_ALARM_PATH_NAS}"
sudo /script/mount_oper.sh mount_bind "${PROTECT_MANAGER_ALARM_PATH}" "${APP_ALARM}"

if [[ ! -d "${PROTECT_MANAGER_KMC_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_KMC_PATH}"
  chmod 750 "${PROTECT_MANAGER_KMC_PATH}"
fi

# PROTECT_MANAGER_KMC_PATH KMC 文件夹 基础设施创建
if [[ ! -f "${PROTECT_MANAGER_KMC_PATH}kmc.properties" ]]; then
  cp /app/conf/wcc/kmc.properties "${PROTECT_MANAGER_KMC_PATH}"
fi

sudo /script/mount_oper.sh mount_bind "${PROTECT_MANAGER_KMC_PATH}" "${APP_CONF_WCC}"

if [[ ! -d "${PROTECT_MANAGER_AGENT_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_AGENT_PATH}"
  chmod 700 "${PROTECT_MANAGER_AGENT_PATH}"
fi
if [[ ! -d "${PROTECT_MANAGER_AGENT_PATH_NAS}" ]]; then
  mkdir "${PROTECT_MANAGER_AGENT_PATH_NAS}"
  chmod 700 "${PROTECT_MANAGER_AGENT_PATH_NAS}"
fi
cp_data_between_two_dir "${PROTECT_MANAGER_AGENT_PATH}" "${PROTECT_MANAGER_AGENT_PATH_NAS}"

if [[ ! -d "${PROTECT_MANAGER_EXPORT_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_EXPORT_PATH}"
fi
if [[ ! -d "${PROTECT_MANAGER_EXPORT_PATH_NAS}" ]]; then
  mkdir "${PROTECT_MANAGER_EXPORT_PATH_NAS}"
fi
if [[ ! -d "${OCEAN_PROTECT_STORAGE_CONFIG_EXPORT_PATH}" ]]; then
  mkdir "${OCEAN_PROTECT_STORAGE_CONFIG_EXPORT_PATH}"
  chmod 770 "${OCEAN_PROTECT_STORAGE_CONFIG_EXPORT_PATH}" -R
fi
cp_data_between_two_dir "${PROTECT_MANAGER_EXPORT_PATH}" "${PROTECT_MANAGER_EXPORT_PATH_NAS}"

sudo /script/mount_oper.sh mount_bind "${PROTECT_MANAGER_AGENT_PATH}" "${APP_AGENT}"


find "${PROTECT_MANAGER_PM_LOG_NODE_PATH}/PM_System_Base" -type d | xargs chmod 750
find "${PROTECT_MANAGER_PM_LOG_NODE_PATH}/PM_System_Base" -type f | xargs chmod 640
find "${PROTECT_MANAGER_SYSBACKUP_PATH}" -type d | xargs chmod 700
find "${PROTECT_MANAGER_SYSBACKUP_PATH}" -type f | xargs chmod 600

find "/opt/OceanProtect/protectmanager/" -type d | grep -vw "/opt/OceanProtect/protectmanager/" | grep -vw "/opt/OceanProtect/protectmanager/kmc" | xargs chmod 750
find "/opt/OceanProtect/protectmanager/" -type f | grep -v "/opt/OceanProtect/protectmanager/kmc/master.ks" | grep -v "/opt/OceanProtect/protectmanager/kmc/backup.ks" | xargs chmod 640


sudo /script/change_permission.sh chmod_group_write /tmp

if [[ -f "/tmp/app.jar" ]]; then
  rm -rf "/tmp/app.jar"
  echo "remove app.jar"
fi

if [[ -d "/tmp/BOOT-INF" ]]; then
  rm -rf "/tmp/BOOT-IN"
  echo "remove BOOT-IN"
fi

deploy_json_dir_alarm_zh="/tmp/BOOT-INF/classes/conf/alarmI18nZ/"
if [ ! -d "$deploy_json_dir_alarm_zh" ];then
   echo "create $deploy_json_dir_alarm_zh"
   mkdir -p  "$deploy_json_dir_alarm_zh"
fi

deploy_json_dir_alarm_en="/tmp/BOOT-INF/classes/conf/alarmI18nE/"
if [ ! -d "$deploy_json_dir_alarm_en" ];then
   echo "create $deploy_json_dir_alarm_en"
   mkdir -p  "$deploy_json_dir_alarm_en"
fi

sudo /script/change_permission.sh change_owner_nobody_nobody  /tmp/BOOT-INF
sudo /script/change_permission.sh chmod /tmp/BOOT-INF 700

cp "/app/app.jar" "/tmp"
cd "/tmp"
sudo /script/change_permission.sh chmod /tmp/app.jar 700

sudo /script/mount_oper.sh mount_bind "/tmp/app.jar" "/app/app.jar"
cd "/app"

if [[ ! -d "${PROTECT_MANAGER_CERT_CA_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CERT_CA_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CERT_CA_CERTS_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CERT_CA_CERTS_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CRL_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CRL_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CERT_INTERNAL_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CERT_INTERNAL_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CERT_INTERNAL_OpenAPI_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CERT_INTERNAL_OpenAPI_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CERT_INTERNAL_Agent_Ca_PATH}" ]]; then
  mkdir -p "${PROTECT_MANAGER_CERT_INTERNAL_Agent_Ca_PATH}"
fi

PM_KEY_STORE_FILE_PATH="${PROTECT_MANAGER_CERT_PATH}/pm.store.p12"
if [[ -e "${PM_KEY_STORE_FILE_PATH}" ]]; then
  chmod 640 "${PM_KEY_STORE_FILE_PATH}"
fi

PM_KEY_STORE_PWD_PATH="${PROTECT_MANAGER_CERT_PATH}/pm.store.p12.cnf"
if [[ -e "${PM_KEY_STORE_PWD_PATH}" ]]; then
  chmod 640 "${PM_KEY_STORE_PWD_PATH}"
fi

chmod 750 "${PROTECT_MANAGER_PATH}"
chmod 750 "${PROTECT_MANAGER_KMC_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_PATH}"
chmod 750 "${PROTECT_MANAGER_SYSBACKUP_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_CA_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_CA_CERTS_PATH}"
chmod 750 "${PROTECT_MANAGER_CRL_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_INTERNAL_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_INTERNAL_OpenAPI_PATH}"
chmod -R 750 "${PROTECT_MANAGER_CERT_INTERNAL_Agent_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_INTERNAL_Agent_Ca_PATH}"
chmod 750 "${PROTECT_MANAGER_DUMP_PATH}"
chmod 750 "${PM_PRIVATE_PATH_NAS}"
chmod -R 700 "${PROTECT_MANAGER_AGENT_LOG_PATH}"
chmod -R 700 "${PROTECT_MANAGER_REPORT_PATH}"


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
    timezoneJson=`sudo /script/curl_dorado_timezone.sh`
    timezones=`getJsonValuesByAwk "${timezoneJson}" "CMO_SYS_TIME_ZONE_NAME" "Asia/Shanghai"`
    echo "local timezone is $timezoneJson"
    echo ${timezones} | tr -d '"' > "${PROTECT_MANAGER_PATH}/timezone"

    sudo /script/mount_oper.sh mount_bind "${PROTECT_MANAGER_PATH}/timezone" "/etc/timezone"
else
    ip="0.0.0.0"
    echo 'Asia/Shanghai' > "${PROTECT_MANAGER_PATH}/timezone"
    sudo /script/mount_oper.sh mount_bind "${PROTECT_MANAGER_PATH}/timezone" "/etc/timezone"
fi

if [ "$DEPLOY_TYPE" = "cloudbackup" ] || [ "$DEPLOY_TYPE" = "d3" ]; then
  JAVA_OPTS="${JAVA_OPTS} -Ddatasource_pool_minimum_idle=5 -Ddatasource_pool_maximum_pool_size=30"
elif [ "$DEPLOY_TYPE" = "d0" ] || [ "$DEPLOY_TYPE" = "d1" ] || [ "$DEPLOY_TYPE" = "d2" ]; then
  JAVA_OPTS="${JAVA_OPTS} -Ddatasource_pool_minimum_idle=10 -Ddatasource_pool_maximum_pool_size=20"
else
  JAVA_OPTS="${JAVA_OPTS} -Ddatasource_pool_minimum_idle=20 -Ddatasource_pool_maximum_pool_size=100"
fi

RUN=1
while [ $RUN -eq 1 ]; do
  if [ ! -f "${APP_JAR}" ]; then
    exit 1
  fi
  "${JAVA_HOME}"/bin/java -Xms2500M -Xmx2500M -Xmn1300M -Xss512K -XX:MaxMetaspaceSize=356M -XX:MaxDirectMemorySize=512M -XX:ReservedCodeCacheSize=512M -XX:+HeapDumpOnOutOfMemoryError -XX:HeapDumpPath="${PROTECT_MANAGER_DUMP_PATH}"/heapdump.hprof -Dserver.address=$ip -Djdk.tls.ephemeralDHKeySize=3072 ${JAVA_OPTS} -jar "${APP_JAR}" &
  wait
done
