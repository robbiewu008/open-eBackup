#!/bin/bash

rootCAFile="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"
tokenFile=$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)
enableOperation="open"
waitingOperation="waiting"

main()
{
    # 查询 sftp_enable 开关为开，则直接启动
    configmap=$(curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer ${tokenFile}" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf)
    sftp_enable=$(echo "${configmap} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('sftp_enable', ''))")
    if [[ "${sftp_enable}" == "open" ]]; then
        echo "sftp_enable switch ${sftp_enable} is true."
        sudo -E /opt/sftp/package/actual_install.sh ${enableOperation}
    fi
    # sftp_enable 开关不开，不拉起其他进程
    if [[ "${sftp_enable}" != "open" ]]; then
        echo "sftp_enable switch ${sftp_enable} is not true."
        sudo -E /opt/sftp/package/actual_install.sh ${waitingOperation}
        while true
            do
                # 检查configmap中检查sftp_enable状态
                configmap=$(curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer ${tokenFile}" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf)
                sftp_enable=$(echo "${configmap} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('sftp_enable', ''))")
                if [[ "${sftp_enable}" != "open" ]]; then
                    echo "sftp_enable switch ${sftp_enable} is not true."
                    sleep 5
                    continue
                fi
                echo "sftp_enable switch is ${sftp_enable}."
                # 如果为开，退出程序，重拉服务，确保dockerfile中的环境变量传递
                exit 1
            done
    fi

    # 适配SFTP关闭，sftp容器重启
    while true
    do
        if [ -f "/opt/script/restart_service" ];then
            exit 1
        fi
        sleep 5
    done
}

main