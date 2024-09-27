#!/bin/bash


cd $(dirname ${BASH_SOURCE[0]})

function usage() {
    echo "
Usage: dp_install.sh COMMAND

Commands:
    install --address [ADDRESS] --type [TYPE]
    reset"
}

function install_dpserver() {
    echo "start install dataprotect deployment server"

    ADDRESS=$1
    TYPE=$2

    # create dpserver user and user group,
    if id dpserver >& /dev/null; then
      echo 'Failed to install dpserver.'
      echo 'The user dpserver already exist.'
      echo 'Please reset before re-install'
      exit 1
    fi
    useradd -m -u 22838 dpserver

    cp -r . /usr/local/dpserver
    chown -R dpserver:dpserver /usr/local/dpserver
    chmod 550 -R /usr/local/dpserver/bin/dpserver
    chmod +x /usr/local/dpserver/scripts/cgroups.sh
    chmod +x /usr/local/dpserver/scripts/add_firewall_ports.sh

    ln -s /usr/local/dpserver/bin/dpserver /usr/bin/dpserver
    chown -R dpserver:dpserver /usr/bin/dpserver
    chmod 550 -R /usr/bin/dpserver

    # verify 工具
    ln -s /usr/local/dpserver/bin/verify_tool /usr/bin/verify_tool
    chown -R dpserver:dpserver /usr/bin/verify_tool
    chmod 555 /usr/bin/verify_tool

    #install dpserver_sudoers
    cp ./conf/dpserver_sudoers /etc/sudoers.d/dpserver_sudoers

    # 替换模板中的ADDRESS
    eval "cat <<EOF
$(< conf/dpserver.service.tpl)
EOF
"  > conf/dpserver.service
    cp conf/dpserver.service /etc/systemd/system

    # 启动服务
    systemctl enable dpserver.service
    systemctl daemon-reload
    systemctl restart dpserver

    echo "Install dataprotect deployment server succeed"
}

function reset_dpserver() {
    echo "start reset dataprotect deployment server"

    if id dpserver >& /dev/null; then userdel -r dpserver; fi

    if systemctl status dpserver >& /dev/null; then
      systemctl stop dpserver;
    fi

    rm -f /usr/bin/dpserver
    rm -f /usr/bin/verify_tool
    rm -rf /usr/local/dpserver
    rm -f /etc/systemd/system/dpserver.service

    systemctl daemon-reload

    echo "reset dataprotect deployment server succeed"
}

function status_ok(){
    # 判断dpserver服务是否启动
    if ! systemctl is-active --quiet dpserver; then
      echo 1
      return
    fi

    # 判断25088端口是否被监听
    if ! netstat -tuln | grep ":25088" >& /dev/null; then
      echo 1
      return
    fi
    echo 0
}

function get_version(){
    manifest_path=$1
    version=$(grep Version < "$manifest_path" | awk '{print $2}' | tr -d '\r')
    echo "$version"
}


function version_ok(){
    expect_version=$1
    run_version=$(get_version /usr/local/dpserver/manifest.yml)
    if [ "$run_version" = "$expect_version" ]; then
      echo 0;
    else
      echo 1;
    fi
}

function  replace_dpserver_from(){
    oring_path=$1
    cd "$oring_path"
    echo "replace dpserver from $oring_path"
    cp -rfT . /usr/local/dpserver
    chown -R dpserver:dpserver /usr/local/dpserver
    chmod 550 /usr/local/dpserver/bin/dpserver
    chmod +x /usr/local/dpserver/scripts/cgroups.sh
    chmod +x /usr/local/dpserver/scripts/add_firewall_ports.sh
    cp ./conf/dpserver_sudoers /etc/sudoers.d/dpserver_sudoers
    cd -
}

function precheck_version_right(){
    package_version=$1
    run_version=$(get_version /usr/local/dpserver/manifest.yml)
    if [[ $package_version > $run_version ]]; then
      return 0
    else
      return 1
    fi
}

function upgrade_dpserver() {
    sleep 5s
    echo "start upgrade dataprotect deployment server"
    UpgradePackageName=$1
    echo PackageName is "$UpgradePackageName"
    version=$(get_version /opt/DataBackup/packages/"$UpgradePackageName"/manifest.yml)
    echo version is "$version"
    # 升级前检查包版本是否大于当前包版本
    precheck_version_right $version
    if [ $? -eq 1 ]; then
      echo "please check package version! package version must greater than running one"
      exit 1
    fi

    # upgrade_status 1表示失败， 0表示成功
    upgrade_status=1

    for (( i=1; i<=3; i++)); do
      # 1. 停止dpserver服务
      sleep 5s
      systemctl stop dpserver
      # 2. 替换新的dpserver包
      replace_dpserver_from /opt/DataBackup/packages/"$UpgradePackageName"

      # 3. 重启dpserver服务
      systemctl restart dpserver
      echo "systemctl restart dpserver now"
      sleep 10s

      # 4. 判断是否升级成功
      check_version_result=$(version_ok "$version")
      check_status_result=$(status_ok)
      if [ "$check_version_result" -eq 0 ]; then
        echo "Version right, successfully upgrade to $version"
      else
        echo "Version wrong"
      fi

      if [ "$check_status_result" -eq 0 ]; then
        echo "Status right, service is running, and dpserver port work"
      else
        echo "Status wrong, new dpserver not working"
      fi

      if [ "$check_version_result" -eq 0 ] && [ "$check_status_result" -eq 0 ]; then
        upgrade_status=0
        break
      fi
    done
    if [ $upgrade_status -ne 0 ]; then
        roll_back_dpserver
        echo "upgrade dataprotect deployment server failed"
        return 1
    fi

    echo "upgrade dataprotect deployment server succeed"
    return 0
}

function roll_back_dpserver(){
    echo "rollback dataprotect deployment server"
    # 替换可执行文件回备份的版本
    replace_dpserver_from /opt/DataBackup/dpserver/backup
    # 重启dpserver服务
    systemctl restart dpserver
}


main() {
    set -e
    if [ $# -lt 1 ]; then
        usage; exit 1
    fi
    CMD=$1

    if [ "$CMD" == "install" ]; then
        if [ $# -lt 5 ]; then
            usage; exit 1
        fi
        install_dpserver $3 $5
    elif [ $CMD == "reset" ]; then
        reset_dpserver
    elif [ $CMD == "upgrade" ]; then
        upgrade_dpserver $2
    else
        echo "unknow command $CMD"
    fi
}

main $@