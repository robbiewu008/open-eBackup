# build AppPlugins_NAS
#！ /bin/bash

CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(dirname ${CURRENT_SCRIPT})
NAS_ROOT_DIR=`cd ${SCRIPT_PATH}/.. && pwd`
SCRIPT_NAME="${CURRENT_SCRIPT##*/}"


MODULE_REPO="ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/Module.git"

if [ -n ${branch} ]; then
    MODULE_BRANCH=${branch}
fi

if [ -z ${MODULE_BRANCH} ]; then
    MODULE_BRANCH="develop_OceanProtect_DataBackup"
fi

function log_echo()
{
    local message="$1"
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][${SCRIPT_NAME}][$(whoami)] ${message}"
}

function clean_pkg()
{
    rm -rf ${NAS_ROOT_DIR}/Module
    log_echo "Finish to clean pkg files"
}

function git_download_code()
{
    local repo="$1"
    if [ -z "${repo}" ];then
        log_echo "not exist the repo[${repo}]"
        return 1
    fi
    local branch="$2"
    git clone "${repo}" -b "${branch}"
    return $?
}

function main()
{
    # 清理目录
    clean_pkg
	
    cd ${NAS_ROOT_DIR}
    git_download_code "${MODULE_REPO}" "${MODULE_BRANCH}"
    cd - >/dev/null
    return $?
}

main "$@"
exit $?