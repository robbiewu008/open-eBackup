#!/bin/sh
set +x

NGINX_ROOT_PATH="${AGENT_ROOT}/nginx/"
NGINX_CONF_PATH="${AGENT_ROOT}/nginx/conf/nginx.conf"

LOG_USER=$LOGNAME
if [ "root" = "${LOG_USER}" ]; then
    echo "You can not execute this script with user root."
    exit 1
fi

main()
{
    if [ ! -d "${NGINX_ROOT_PATH}" ]; then
        return 1
    fi

    cd "${NGINX_ROOT_PATH}"
    ${NGINX_ROOT_PATH}/rdnginx -c ${NGINX_CONF_PATH} -s reload
    return 0
}

main
exit $?