#!/bin/sh
set +x

###### Input Parameter ######
SQLITE_BACKUP_FILE_DIR=$1 # 待升级的数据库路径
SQLITE_TARGET_FILE_DIR=$2 # 更新包数据库路径

###### params ######
AGENT_ROOT_PATH=${AGENT_ROOT}
LOG_FILE_NAME=${AGENT_ROOT_PATH}/log/agent_upgrade_sqlite.log

###### old version information ######
OLD_VERSION="" # "1.9.0" "1.9.1" "1.9.2"
OLD_UPGRADE_DIR=""
OLD_DEFAULT_VERSION="1.9.0"

###### upgrade version information ######
UPGRADE_VERSION="" # 1.9.3
UPGRADE_SUPPORT_VERSION="" # "1.9.0" "1.9.1" "1.9.1,1.9.2"
UPGRADE_SQL_FILE=""

SYS_NAME=`uname -s`
if [ "${SYS_NAME}" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi

Log()
{
    if [ ! -f "${LOG_FILE_NAME}" ]; then
        touch "${LOG_FILE_NAME}"
        chmod 600 "${LOG_FILE_NAME}"
    fi

    DATE=`date +%y-%m-%d--%H:%M:%S`
    USER_NAME=`whoami`
    echo "[${DATE}][$$][${USER_NAME}] $1" >> "${LOG_FILE_NAME}"
}

GetUpgradeVersion()
{
    Log "Check upgrade information."
    # 1. Checking the upgrade directory
    typeset upgradeDirSize=`ls -lr ${SQLITE_TARGET_FILE_DIR} 2>/dev/null | grep "^d" | ${AWK} '{print $NF}' | grep "^upgrade" |wc -l`
    if [ ${upgradeDirSize} -ne 1 ]; then
        Log "The upgrade directory cannot be found."
        return 1
    fi
    typeset upgradeDir=`ls -lr ${SQLITE_TARGET_FILE_DIR} 2>/dev/null | grep "^d" | ${AWK} '{print $NF}' | grep "^upgrade"`
    Log "Check upgrade dir[$upgradeDir] succ."

    # 2. Checking the upgrade sql file
    if [ ! -f "${SQLITE_TARGET_FILE_DIR}/${upgradeDir}/upgrade.sql" ]; then
        Log "The upgrade sql file cannot be found."
        return 1
    fi
    UPGRADE_SQL_FILE="${SQLITE_TARGET_FILE_DIR}/${upgradeDir}/upgrade.sql"
    Log "Check upgrade sql file succ."

    # 3. Checking the upgrade attribute file
    if [ ! -f "${SQLITE_TARGET_FILE_DIR}/${upgradeDir}/attribute.conf" ]; then
        Log "The upgrade attribute file cannot be found."
        return 1
    fi
    UPGRADE_VERSION=`cat ${SQLITE_TARGET_FILE_DIR}/${upgradeDir}/attribute.conf | grep ^version | ${AWK} -F '=' '{print $NF}'`
    UPGRADE_SUPPORT_VERSION=`cat ${SQLITE_TARGET_FILE_DIR}/${upgradeDir}/attribute.conf | grep ^support_version | ${AWK} -F '=' '{print $NF}'`
    Log "Check upgrade attribute file succ."
    Log "New version=${UPGRADE_VERSION}, support version=${UPGRADE_SUPPORT_VERSION}."

    Log "Check upgrade information end."
    return 0
}

GetCurrentVersion()
{
    Log "Check old information."
    # 1. Checking the upgrade directory
    typeset upgradeDirSize=`ls -lr ${SQLITE_BACKUP_FILE_DIR} 2>/dev/null | grep "^d" | ${AWK} '{print $NF}' | grep "^upgrade" | wc -l`
    if [ ${upgradeDirSize} -ne 1 ]; then
        Log "The old upgrade directory cannot be found.Use the default version=${OLD_DEFAULT_VERSION}."
        OLD_VERSION="${OLD_DEFAULT_VERSION}"
        return 0
    fi
    OLD_UPGRADE_DIR=`ls -lr ${SQLITE_BACKUP_FILE_DIR} 2>/dev/null | grep "^d" | ${AWK} '{print $NF}' | grep "^upgrade"`
    Log "Check old dir[${OLD_UPGRADE_DIR}] succ."

    # 2. Checking the sql file
    if [ ! -f "${SQLITE_BACKUP_FILE_DIR}/${OLD_UPGRADE_DIR}/upgrade.sql" ]; then
        Log "The old sql file cannot be found."
        return 1
    fi
    Log "Check old sql file succ."

    # 3. Checking the attribute file
    if [ ! -f "${SQLITE_BACKUP_FILE_DIR}/${OLD_UPGRADE_DIR}/attribute.conf" ]; then
        Log "The old attribute file cannot be found."
        return 1
    fi
    Log "Checking old attribute file succ."
    OLD_VERSION=`cat ${SQLITE_BACKUP_FILE_DIR}/${OLD_UPGRADE_DIR}/attribute.conf | grep ^version | ${AWK} -F '=' '{print $NF}'`
    Log "Obtaining the old version[$OLD_VERSION] succ."

    Log "Check old information end."
    return 0
}

VerifyUpgradeVersion()
{
    Log "Verify upgrade version."
    typeset oldIFS="$IFS"
    IFS=","
    typeset versionArr=$UPGRADE_SUPPORT_VERSION
    matchVersion=""
    for version in ${versionArr}
    do
        if [ "${version}" = "${OLD_VERSION}" ]; then
            Log "Version matching succeeded."
            matchVersion=${version}
            break
        fi
    done
    IFS=$OLD_IFS
    if [ -n "${matchVersion}" ]; then
        return 0
    fi
    return 1
}

CheckUpgradeAvailable()
{
    Log "Start check params."

    # 1. Checking the upgrade information
    GetUpgradeVersion
    if [ $? -ne 0 ]; then
        Log "Failed to check the upgrade information. The package is incomplete."
        return 1
    fi

    # 2. Checking the current information
    GetCurrentVersion
    if [ $? -ne 0 ]; then
        Log "Failed to check the current information. "
        return 1
    fi

    # 3. Verify the upgrade version
    VerifyUpgradeVersion
    if [ $? -ne 0 ]; then
        Log "Failed to verify the upgrade version."
        return 1
    fi
    return 0
}


RunUpgradeSql()
{
    Log "Start upgrade."
    # 1. Backup database files
    cp -a ${SQLITE_BACKUP_FILE_DIR}/AgentDB.db ${SQLITE_BACKUP_FILE_DIR}/AgentDB-bak.db

    # 2. exec sql
    sqlite3 ${SQLITE_BACKUP_FILE_DIR}/AgentDB.db "`cat ${UPGRADE_SQL_FILE}`"
    if [ $? -ne 0 ]; then
        Log "Failed exec ${UPGRADE_SQL_FILE}."
        rm -rf "${SQLITE_BACKUP_FILE_DIR}"/AgentDB.db
        mv -f ${SQLITE_BACKUP_FILE_DIR}/AgentDB-bak.db ${SQLITE_BACKUP_FILE_DIR}/AgentDB.db
        return 1
    fi
    Log "The sql is executed successfully.."

    rm ${SQLITE_BACKUP_FILE_DIR}/AgentDB-bak.db
    cp -f "${SQLITE_BACKUP_FILE_DIR}/AgentDB.db" "${SQLITE_TARGET_FILE_DIR}"
    return 0
}

Upgrade()
{
    Log "Start upgrade."
    CheckUpgradeAvailable
    if [ $? -eq 0 ]; then
        RunUpgradeSql
        if [ $? -ne 0 ]; then
            Log "Failed to upgrade the database."
            exit 1
        fi
    else
        if [ "${OLD_VERSION}" != "${UPGRADE_VERSION}" ]; then
            Log "Failed to upgrade the database."
            exit 1
        fi

        Log "Upgrade with the same version."
        cp -r "${SQLITE_BACKUP_FILE_DIR}/"*.db "${SQLITE_TARGET_FILE_DIR}" > /dev/null 2>&1
    fi
}

Upgrade

Log "The database is upgraded successfully."

exit 0