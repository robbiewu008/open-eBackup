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
    ${AGENT_ROOT}/bin/agent_stop.sh nginx

    ${AGENT_ROOT}/bin/agent_start.sh nginx
    return $?
}

main
exit $?