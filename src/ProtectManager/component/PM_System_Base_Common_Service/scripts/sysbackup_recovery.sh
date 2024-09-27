#!/bin/bash

#########################################
# Copyright (c) 2023-2023 Huawei .
# All rights reserved.
#
# Please send feedback to http://www.huawei.com
#
# Function 管理数据备份恢复脚本
# revise note
########################################

LOG_PATH='/var/log/sysbackup_recovery.log'

# 下载sftp文件
sftp_download()
{
expect <<- EOF
set timeout 5
spawn sftp "${1}@${2}"
expect {
"*yes/no*" {send "yes\r"; exp_continue}
"*password:" {send -- "${3}\r"}
}
expect "sftp*"
send -- "mget ${4} ${5}\r"
expect "sftp*"
send "bye\r"
EOF
}

# 获取sftp文件列表
sftp_ls()
{
expect <<- EOF
set timeout 5
spawn sftp "${1}@${2}"
expect {
"*yes/no*" {send "yes\r"; exp_continue}
"*password:" {send -- "${3}\r"}
}
expect "sftp*"
send -- "ls -l ${4}\r"
expect "sftp*"
send "bye\r"
EOF
}

# 打印错误日志
log_error()
{
  time=$(date "+%Y-%m-%d %H:%M:%S")
  user=$(whoami)
  echo -e "\033[31m[${time}][USER:${user}][ERROR] ${FUNCNAME[1]}(), line ${BASH_LINENO[0]}: $*\033[0m" | tee -a $LOG_PATH
  echo -e "\033[31mYou can get details in: /var/log/sysbackup_recovery.log\033[0m"
}

# 打印日志
log_info()
{
  time=$(date "+%Y-%m-%d %H:%M:%S")
  user=$(whoami)
  echo -e "\033[32m[${time}][USER:${user}][INFO] ${FUNCNAME[1]}(): $*\033[0m" | tee -a $LOG_PATH
}

# 删除recovery文件夹
clean_recovery_dir()
{
  rm -rf "$recovery_root_path"
  rm -rf "$decrypt_path"
}

# 1、准备恢复环境
prepare_for_recovery()
{
  log_info "Start prepare dir for recovery."
  recovery_root_path='/opt/cyberengine/comm_data/protectmanager/sysbackup/recovery'
  if [ -d $recovery_root_path ]; then
    rm -rf $recovery_root_path
  fi
  mkdir -p $recovery_root_path
  chown 99:99 $recovery_root_path
  decrypt_path='/opt/cyberengine/comm_data/protectmanager/sysbackup/decrypt'
  if [ -d $decrypt_path ]; then
    rm -rf $decrypt_path
  fi
  mkdir -p $decrypt_path
  chown 99:99 $decrypt_path
  log_info "Prepared dir for recovery success."
}

# 2、下载最新备份副本
download_backup_archived()
{
  backup_bag='sftp'
  log_info "Start download backup from SFTP."
  if [ -z "$sftp_copy_name" ]; then
    file_list=$decrypt_path/file_list.txt
    if [ -e $file_list ]; then
      rm $file_list
    fi
    # 获取最新备份
    sftp_ls "$sftp_user" "$sftp_host" "$sftp_password" "$sftp_path" > $file_list 2>/dev/null
    backup_file_name=$(< $file_list grep '.zip' | awk '{print $9}' | sort -r | head -1 | tr -d '\r')
    rm $file_list
    if [ -n "$backup_file_name" ]; then
      log_info "Get the latest backup name success: ${backup_file_name}"
    else
      log_error "Get the latest backup name failed!"
      exit 1
    fi
  else
    backup_file_name=$sftp_copy_name
  fi
  # 下载备份文件
  sftp_download "$sftp_user" "$sftp_host" "$sftp_password" "$sftp_path"/"$backup_file_name" $decrypt_path >/dev/null 2>&1
  if [ -e "${decrypt_path}/${backup_file_name}" ]; then
    log_info "Download the backup success: ${backup_file_name}"
  else
    log_error "Download the backup failed!"
    exit 1
  fi
}

# 3、校验备份副本的正确性.
check_recovery_file()
{
  log_info "Start check recovery file."
  # 1)、检查文件大小是否超过4G
  copy_name=$decrypt_path/$backup_file_name
  copy_size=$(find "$copy_name" -type f -exec ls -l {} \; | awk '{print $5}')
  max_size=$((4*1024*1024*1024))
  if [ "$copy_size" -gt "$max_size" ]; then
    log_error "The zip file size greater than 4G!"
    clean_recovery_dir
    exit 1
  fi
  # 2)、检查zip文件是否受损
  zip_errors=$(unzip -t "$copy_name" | grep 'No errors detected in compressed data')
  if [ -z "$zip_errors" ]; then
      log_error "The zip file has destroyed!"
      clean_recovery_dir
      exit 1
  fi
  # 3)、拦截zip炸弹
  zip_entries=$(unzip -t "$copy_name" | wc -l)
  max_entries_set=100
  if [ "$zip_entries" -gt "$max_entries_set" ]; then
    log_error "The zip entries exceed 100!"
    clean_recovery_dir
    exit 1
  fi
  # 4)、查看上传文件是否齐全
  summary_file_name="summary.properties"
  protect_manager_zip_file_name="pm.zip"
  infrastructure_zip_file_name="infrastructure.zip"
  sm_result=$(unzip -l "$copy_name" | grep $summary_file_name)
  if [ -z "$sm_result" ]; then
      log_error "The zip file not completed, missing: ""$summary_file_name"
      clean_recovery_dir
      exit 1
  fi
  pm_result=$(unzip -l "$copy_name" | grep $protect_manager_zip_file_name)
  if [ -z "$pm_result" ]; then
      log_error "The zip file not completed, missing: ""$protect_manager_zip_file_name"
      clean_recovery_dir
      exit 1
  fi
  infra_result=$(unzip -l "$copy_name" | grep $infrastructure_zip_file_name)
  if [ -z "$infra_result" ]; then
      log_error "The zip file not completed, missing: ""$infrastructure_zip_file_name"
      clean_recovery_dir
      exit 1
  fi
  # 5)、检查一下summary.properties文件内容是否正确
  if [ -e $decrypt_path/$summary_file_name ]; then
    rm $decrypt_path/$summary_file_name
  fi
  summary_properties_key_list=('time' 'size' 'desc' 'isAuto' 'version' 'infraMac' 'pmMac' 'deployType')
  unzip -j "$copy_name" "$(unzip -l "$copy_name" | grep $summary_file_name | awk '{print $4}')" -d $decrypt_path >$LOG_PATH 2>&1
  for element in "${summary_properties_key_list[@]}"
  do
  res=$(< $decrypt_path/$summary_file_name grep "$element")
  if [ "$res" == "" ]; then
    log_error "The summary file missing field: ""$element"
    clean_recovery_dir
    exit 1
  fi
  done
  # 6)、检查文件和当前部署类型是否匹配
  deploy_type=$(< $decrypt_path/$summary_file_name grep "${summary_properties_key_list[7]}" | cut -d'=' -f2)
  if [ "$deploy_type" != "d5" ]; then
    log_error "The current deploy type is not OceanCyber!"
    clean_recovery_dir
    exit 1
  fi
  # 7)、描述信息长度限制在255以下
  desc_length=$(< $decrypt_path/$summary_file_name grep "${summary_properties_key_list[2]}" | cut -d'=' -f2 | wc -L)
  if [ "$desc_length" -gt 255 ]; then
    log_error "The field length of 'desc' is greater than 255!"
    clean_recovery_dir
    exit 1
  fi
  # 8)、是否自动需要是true或者false
  is_auto=$(< $decrypt_path/$summary_file_name grep "${summary_properties_key_list[3]}" | cut -d'=' -f2)
  if [ "$is_auto" != "true" ] && [ "$is_auto" != "false" ] && [ "$is_auto" != "upgrade" ]; then
    log_error "The field of 'is_auto' is illegal!"
    clean_recovery_dir
    exit 1
  fi
  # 9)、查看文件是否被修改过
  if [ -e "$decrypt_path"/"$infrastructure_zip_file_name" ]; then
    rm "$decrypt_path"/"$infrastructure_zip_file_name"
  fi
  if [ -e "$decrypt_path"/"$protect_manager_zip_file_name" ]; then
    rm "$decrypt_path"/"$protect_manager_zip_file_name"
  fi
  unzip -j "$copy_name" "$(unzip -l "$copy_name" | grep $infrastructure_zip_file_name | awk '{print $4}')" -d "$decrypt_path" >$LOG_PATH 2>&1
  unzip -j "$copy_name" "$(unzip -l "$copy_name" | grep $protect_manager_zip_file_name | awk '{print $4}')" -d "$decrypt_path" >$LOG_PATH 2>&1
  infra_mac=$(< $decrypt_path/$summary_file_name grep "${summary_properties_key_list[5]}" | cut -d'=' -f2)
  pm_mac=$(< $decrypt_path/$summary_file_name grep "${summary_properties_key_list[6]}" | cut -d'=' -f2)
  if [ "$infra_mac" != "$(sha512sum $decrypt_path/$infrastructure_zip_file_name | cut -d" " -f1)" ] || [ "$pm_mac" != "$(sha512sum $decrypt_path/$protect_manager_zip_file_name | cut -d" " -f1)" ]; then
    log_error "The zip files have modified!"
    clean_recovery_dir
    exit 1
  fi
  # 10)、检查是否跨版本恢复
  OLD_IFS=$IFS
  IFS=$'\n'
  for element in $(helm list -A --deployed | tail -n +2)
  do
    name=$(echo "$element" | awk '{print $1}')
    if helm get values "$name" -n dpa | grep -qw 'deploy_type: d5'; then
      cur_app_version=$(echo "$element" | awk '{print $10}')
      break
    fi
  done
  IFS=$OLD_IFS
  backup_file_version=$(< $decrypt_path/$summary_file_name grep "${summary_properties_key_list[4]}" | cut -d'=' -f2)
  if [ "$cur_app_version" == "$backup_file_version" ]; then
      log_info "Version check success: $cur_app_version"
  else
      log_error "Version check failed! Current version: $cur_app_version, Backup version: $backup_file_version"
      clean_recovery_dir
      exit 1
  fi
  log_info "Recovery file checked pass."
  rm $decrypt_path/$summary_file_name
  rm "$decrypt_path"/"$infrastructure_zip_file_name"
  rm "$decrypt_path"/"$protect_manager_zip_file_name"
}

# 4、解压备份文件
# 前置步骤已解压所有所需子文件
unpack_all_recovery_file()
{
  unzip "$copy_name" -d "$decrypt_path" >$LOG_PATH 2>&1
  chown -R 99:99 $decrypt_path/"$(basename "$copy_name" .zip)"
  log_info "The zip files have unzipped into path: ${decrypt_path}"
}

# 5、解密文件，准备恢复文件
decrypt_all_file()
{
  if [ "${backup_bag}X" == "localX" ]; then
      log_info "Local backup no need decrypt."
      if [ -L $decrypt_path/$protect_manager_zip_file_name ] || [ -L $decrypt_path/$infrastructure_zip_file_name ] || [ -L $recovery_root_path ]; then
            log_error "Can not deal with link symbol!"
            clean_recovery_dir
            exit 1
      fi
      pm_cp_result=$(cp -fpP $decrypt_path/$protect_manager_zip_file_name $recovery_root_path 2>&1)
      infra_cp_result=$(cp -fpP $decrypt_path/$infrastructure_zip_file_name $recovery_root_path 2>&1)
      if [ -n "$pm_cp_result" ]; then
        log_error "Get local backup failed: $pm_cp_result"
        clean_recovery_dir
        exit
      fi
      if [ -n "$infra_cp_result" ]; then
        log_error "Get local backup failed: $infra_cp_result"
        clean_recovery_dir
        exit
      fi
  else
      log_info "Start decrypt zip files."
      # 读取用户口令
      read -rsp "Enter the decrypt password: " decrypt_password
      echo -e "\r"
      # 校验口令长度
      if [ ${#decrypt_password} -gt 16 ] || [ ${#decrypt_password} -lt 8 ]; then
          log_error "Password length must greater then 8 and less than 16!"
          clean_recovery_dir
          exit 1
      fi
      decrypt_by_pm "$decrypt_password"
      log_info "The zip files have decrypted into path: ${recovery_root_path}"
  fi
}
decrypt_by_pm()
{
  pm_system_base_container_name='k8s_pm-system-base_protectmanager-system-base-0_dpa_'
  container_id=$(isula ps | grep $pm_system_base_container_name | awk '{print $1}')
  if [ -z "$container_id" ]; then
    log_error 'No pm-system-base container founded!'
    clean_recovery_dir
    exit 1
  fi
  kmc_pass=$(kubectl exec -it -n dpa infrastructure-0 -c om -- env COLUMNS=`tput cols` LINES=`tput lines` bash -c "grep ssl.key.password /opt/third_data/kafka/config/server.properties | cut -d'=' -f2 | tr -d '\n'")
  if [ -z "$kmc_pass" ]; then
    log_error "Get kmc pass failed!"
    clean_recovery_dir
    exit 1
  fi
  if [ "${#kmc_pass}" -gt 8 ]; then
    log_error "Get kmc pass failed!"
    clean_recovery_dir
    exit 1
  fi
  pure_kmc_pass=\'${kmc_pass}\'
  log_info "Get kmc_pass success."
  request_body=\'\{\"password\":\"${1}\"\}\'
  result=$(isula exec -it "$container_id" /bin/bash -c "curl -X 'POST' -H 'accept: */*' -H 'Content-Type: application/json' --cert /opt/OceanProtect/infrastructure/cert/internal/internal.crt.pem --key /opt/OceanProtect/infrastructure/cert/internal/internal.pem --pass $pure_kmc_pass --cacert /opt/OceanProtect/infrastructure/cert/internal/ca/ca.crt.pem -d $request_body 'https://pm-system-base.dpa.svc.cluster.local:30081/v1/internal/sysbackup/decrypt'")
  if [ -n "${result}" ]; then
    log_error "Decrypt failed!"
    clean_recovery_dir
    exit 1
  fi
}
# 6、解压子系统文件
unzip_sub_system_file()
{
  log_info "Start unzip sub system file."
  decrypted_infra_unzip_result=$(unzip -o -q $recovery_root_path/$infrastructure_zip_file_name -d $recovery_root_path | grep 'cannot find zipfile')
  if [ -n "$decrypted_infra_unzip_result" ]; then
      log_error "The zip file unzip failed: ${recovery_root_path}/${infrastructure_zip_file_name}"
      clean_recovery_dir
      exit 1
  else
      log_info "The sub files of infrastructure has unzipped."
  fi
  decrypted_pm_unzip_result=$(unzip -o -q $recovery_root_path/$protect_manager_zip_file_name -d $recovery_root_path | grep 'cannot find zipfile')
  if [ -n "$decrypted_pm_unzip_result" ]; then
      log_error "The zip file unzip failed: ${recovery_root_path}/${protect_manager_zip_file_name}"
      clean_recovery_dir
      exit 1
  else
      log_info "The sub files of pm has unzipped."
  fi
}

check_reload()
{
  recovery_mk_sha256=$(sha256sum $recovery_root_path/pm/kmc/master.ks | cut -d' ' -f1)
  if [ -z "$recovery_mk_sha256" ]; then
    log_error "Can not find master.ks in backup files!"
    clean_recovery_dir
    exit 1
  fi
  current_mk_sha256=$(sha256sum /opt/cyberengine/comm_data/protectmanager/kmc/master.ks | cut -d' ' -f1)
  if [ -z "$current_mk_sha256" ]; then
    log_error "Can not find master.ks in use, please wait a moment and recovery again or contact technical support!"
    clean_recovery_dir
    exit 1
  fi
  if [ "$recovery_mk_sha256" != "$current_mk_sha256" ]; then
    log_error "Do not support recover after reinstall or recover across devices!"
    clean_recovery_dir
    exit 1
  fi
}
# 7、调用子系统恢复
invoke_sub_system_recovery()
{
  log_info "Start invoke sub system recovery."

  # 校验是否当前环境重装或备份包来自其他设备
  check_reload

  om_infrastructure_container_name='k8s_om_infrastructure-0_dpa_'
  container_id=$(isula ps | grep $om_infrastructure_container_name | awk '{print $1}')
  if [ -z "$container_id" ]; then
    log_error 'No om_infrastructure container founded!'
    clean_recovery_dir
    exit 1
  fi
  # 1）恢复pm
  # 恢复master.ks
  source_file=${recovery_root_path}'/pm/kmc/master.ks'
  target_file='/opt/cyberengine/comm_data/protectmanager/kmc/master.ks'
  if [ -L $source_file ] || [ -L $target_file ]; then
    log_error "Recover master.ks failed: can not deal with link symbol."
    clean_recovery_dir
    exit 1
  fi
  cp_result=$(cp -frP "$source_file" "$target_file" 2>&1)
  if [ -n "$cp_result" ]; then
    log_error "Recover master.ks failed: ${cp_result}"
    clean_recovery_dir
    exit 1
  fi
  find $target_file -type f \( -exec chmod 640 {} \; -o -exec true \; \) -exec chown 99:99 {} \;
  # 恢复cert
  source_dir=${recovery_root_path}'/pm/cert'
  target_dir='/opt/cyberengine/comm_data/protectmanager'
  if [ -L $source_dir ] || [ -L $target_dir ]; then
    log_error "Recover cert failed: can not deal with link symbol."
    clean_recovery_dir
    exit 1
  fi
  cp_result=$(cp -frP -f $source_dir $target_dir 2>&1)
  if [ -n "$cp_result" ]; then
    log_error "Recover cert failed: ${cp_result}"
    clean_recovery_dir
    exit 1
  fi
  find ${target_dir}/cert -type f \( -exec chmod 640 {} \; -o -exec true \; \) -exec chown 99:99 {} \;
  find ${target_dir}/cert -type d \( -exec chmod 750 {} \; -o -exec true \; \) -exec chown 99:99 {} \;
  # 恢复backup.ks
  name="kmc-store-conf"
  namespace="dpa"
  BACKUPKSINFO=$(< /opt/cyberengine/comm_data/protectmanager/sysbackup/recovery/pm/kmc_conf/backup.ks base64 | tr -d '\n')
  tokenFile="/var/run/secrets/kubernetes.io/serviceaccount/token"
  KUBE_CACERT_PATH="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"
  host=$(isula exec -it "$container_id" /bin/bash -c "env | grep -w 'KUBERNETES_SERVICE_HOST' | cut -d'=' -f2 | tr -d '\n'")
  port=$(isula exec -it "$container_id" /bin/bash -c "env | grep -w 'KUBERNETES_SERVICE_PORT' | cut -d'=' -f2 | tr -d '\n'")
  Host="https://"$host:$port
  TOKEN=$(isula exec -it "$container_id" /bin/bash -c "cat $tokenFile | tr -d '\n'")
  DATA=\''{"binaryData":{"backup.ks": "'${BACKUPKSINFO}'"}}'\'
  log_info "Modify ${name}"
  if [ "${BACKUPKSINFO}X" == "X" ]; then
    log_error "Modify ${name} failed, parameter is no exit"
    clean_recovery_dir
    exit 1
  fi
  result=$(isula exec -it "$container_id" /bin/bash -c "curl --cacert $KUBE_CACERT_PATH -X PATCH -H \"Authorization: Bearer ${TOKEN}\" ${Host}/api/v1/namespaces/${namespace}/configmaps/${name} -H \"Content-Type: application/strategic-merge-patch+json\" --data ${DATA}")
  check_result=$(echo "${result}" | grep "Failure")
  if [ -n "${check_result}" ]; then
    log_error "Get ${name} failed!"
    clean_recovery_dir
    exit 1
  fi
  log_info "Modify ${name} success"
  # 2）恢复infrastructure
  internal_cert_infra_path='/opt/OceanProtect/infrastructure/cert/internal/internal.crt.pem'
  internal_key_infra_path='/opt/OceanProtect/infrastructure/cert/internal/internal.pem'
  internal_ca_cert_infra_path='/opt/OceanProtect/infrastructure/cert/internal/ca/ca.crt.pem'
  kmc_pass=$(kubectl exec -it -n dpa infrastructure-0 -c om -- env COLUMNS=`tput cols` LINES=`tput lines` bash -c "grep ssl.key.password /opt/third_data/kafka/config/server.properties | cut -d'=' -f2 | tr -d '\n'")
  if [ -z "$kmc_pass" ]; then
    log_error "Get kmc pass failed!"
    clean_recovery_dir
    exit 1
  fi
  if [ "${#kmc_pass}" -gt 8 ]; then
    log_error "Get kmc pass failed!"
    clean_recovery_dir
    exit 1
  fi
  pure_kmc_pass=\'${kmc_pass}\'
  log_info "Get kmc_pass success."
  result=$(isula exec -it "$container_id" /bin/bash -c "curl -X 'GET' -H 'accept: */*' -H 'Content-Type: application/json' --cert $internal_cert_infra_path --key $internal_key_infra_path --pass $pure_kmc_pass --cacert $internal_ca_cert_infra_path 'https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/data/recover?subsystem=INFRA&data_type=DB&path=/opt/OceanProtect/protectmanager/sysbackup/recovery/infrastructure'" | grep 'success' | cut -d':' -f2 | tr -d '\r')
  if [ "$result" = ' true' ]; then
    log_info "Recover infrastructure DB success."
  else
    log_error "Recover infrastructure DB failed."
    clean_recovery_dir
    exit 1
  fi
  result=$(isula exec -it "$container_id" /bin/bash -c "curl -X 'GET' -H 'accept: */*' -H 'Content-Type: application/json' --cert $internal_cert_infra_path --key $internal_key_infra_path --pass $pure_kmc_pass --cacert $internal_ca_cert_infra_path 'https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/data/recover?subsystem=INFRA&data_type=CONFIG&path=/opt/OceanProtect/protectmanager/sysbackup/recovery/infrastructure'" | grep 'success' | cut -d':' -f2 | tr -d '\r')
  if [ "$result" = ' true' ]; then
    log_info "Recover infrastructure CONFIG success."
  else
    log_error "Recover infrastructure CONFIG failed."
    clean_recovery_dir
    exit 1
  fi
}

# 8、重启Pods
reboot_pm_system()
{
  log_info "Start reboot PM pod."
  result=$(isula exec -it "$container_id" /bin/bash -c "curl -X 'POST' -H 'accept: */*' -H 'Content-Type: application/json' --cert $internal_cert_infra_path --key $internal_key_infra_path --pass $pure_kmc_pass --cacert $internal_ca_cert_infra_path 'https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/pod/delete?moduleName=PM'" | grep 'success' | cut -d':' -f2 | tr -d '\r')
  if [ "$result" = ' true' ]; then
    log_info "Rebooting PM pod, you can use 'kubectl get po -A' to check the reboot progress."
  else
    log_error "Reboot PM pod failed."
    clean_recovery_dir
    exit 1
  fi
}

# 获取本地备份
get_local_backup()
{
  backup_bag='local'
  log_info "Start get backup from local."
  local_backup_path='/opt/cyberengine/comm_data/protectmanager/sysbackup/temp'
  backup_file_list=$(find ${local_backup_path}/*.zip -type f -exec ls -t {} \; | cut -d' ' -f6,8,9,10 2>/dev/null)
  if [ ! -d "$local_backup_path" ]; then
      log_error "Can not found the local backup path!"
      clean_recovery_dir
      exit 1
  fi
  if [ -z "$backup_file_list" ]; then
      log_error "There are no local backup file founded!"
      clean_recovery_dir
      exit 1
  fi
  echo -e "\033[36mBackup File List:\033[0m"
  for element in "${backup_file_list[@]}"
  do
    echo -e "\033[36m${element}\033[0m"
  done
  read -rp "Please choose the backup file to recovery: " backup_file_name
  while ! echo "$backup_file_list" | grep -qx "$backup_file_name"
  do
    read -rp "Bad input, please choose an existed backup file: " backup_file_name
  done
  backup_file_name=$(echo "$backup_file_name" | cut -d'/' -f8)
  decrypt_path='/opt/cyberengine/comm_data/protectmanager/sysbackup/decrypt'
  if [ -L $local_backup_path/"$backup_file_name" ] || [ -L $decrypt_path ]; then
      log_error "Can not deal with link symbol!"
      clean_recovery_dir
      exit 1
  fi
  cp_result=$(cp -fpP $local_backup_path/"$backup_file_name" $decrypt_path 2>&1)
  if [ -n "$cp_result" ]; then
      log_error "Get local backup failed: $cp_result"
      clean_recovery_dir
      exit
  fi
  log_info "Get backup file from local: ${backup_file_name}"
}

# 特殊字符转义，防止send命令注入
transfer_special_characters_for_expect_send()
{
  # 防止expect-send注入，需转义以下字符：\ $ [ ] { } " ` ! @
  echo "$1" | sed 's?\\?\\\\?g;s?\$?\\\$?g;s?\[\\[?g;s?\]?\\]?g;s?{?\\{?g;s?}?\\}?g;s?"?\\\"?g;s?`?\\`?g;s?!?\\!?g;s?@?\\@?g;'
}

# 检查ipv4地址合法性
check_ipv4()
{
  ipStr="$1"
  isValid=$(echo "${ipStr}"|grep -E "^([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])$" | wc -l);
  if [ "${isValid}" -gt 0 ];then
    return 0
  fi
  return 1
}

# 检查ipv6地址合法性
check_ipV6()
{
  if echo "$1" | grep -E '^((([0-9a-fA-F]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9a-fA-F]{1,4}:){6}((([0-9]{1,3}\.){3}[0-9]{1,4})|([0-9a-fA-F]{1,4})|:))|(([0-9a-fA-F]{1,4}:){5}:((([0-9a-fA-F]{1,4})?)|([0-9a-fA-F]{1,4}:[0-9a-fA-F]{1,4})|(([0-9]{1,3}\.){3}[0-9]{1,4})))|(([0-9a-fA-F]{1,4}:){4}:((([0-9a-fA-F]{1,4})?)|(([0-9a-fA-F]{1,4}:){1,2}[0-9a-fA-F]{1,4})|(([0-9a-fA-F]{1,4}:)?([0-9]{1,3}\.){3}[0-9]{1,4})))|(([0-9a-fA-F]{1,4}:){3}:((([0-9a-fA-F]{1,4}:){0,2}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,3}[0-9a-fA-F]{1,4})|$))|(([0-9a-fA-F]{1,4}:){2}:((([0-9a-fA-F]{1,4}:){0,3}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,4}[0-9a-fA-F]{1,4})|$))|(([0-9a-fA-F]{1,4}:){1}:((([0-9a-fA-F]{1,4}:){0,4}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,5}[0-9a-fA-F]{1,4})|$))|(::((([0-9a-fA-F]{1,4}:){0,5}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,6}[0-9a-fA-F]{1,4})|$)))$' >$LOG_PATH 2>&1;then
    return 0
  fi
  return 1
}

gen_default_sftp_path()
{
  pm_system_base_container_name='k8s_pm-system-base_protectmanager-system-base-0_dpa_'
  container_id=$(isula ps | grep $pm_system_base_container_name | awk '{print $1}')
  if [ -z "$container_id" ]; then
    log_error 'No pm-system-base container founded!'
    clean_recovery_dir
    exit 1
  fi
  kmc_pass=$(kubectl exec -it -n dpa infrastructure-0 -c om -- env COLUMNS=`tput cols` LINES=`tput lines` bash -c "grep ssl.key.password /opt/third_data/kafka/config/server.properties | cut -d'=' -f2 | tr -d '\n'")
  if [ -z "$kmc_pass" ]; then
    log_error "Get kmc pass failed!"
    clean_recovery_dir
    exit 1
  fi
  if [ "${#kmc_pass}" -gt 8 ]; then
    log_error "Get kmc pass failed!"
    clean_recovery_dir
    exit 1
  fi
  pure_kmc_pass=\'${kmc_pass}\'
  log_info "Get kmc_pass success."
  result=$(isula exec -it "$container_id" /bin/bash -c "curl -X 'GET' -H 'accept: */*' -H 'Content-Type: application/json' --cert /opt/OceanProtect/infrastructure/cert/internal/internal.crt.pem --key /opt/OceanProtect/infrastructure/cert/internal/internal.pem --pass $pure_kmc_pass --cacert /opt/OceanProtect/infrastructure/cert/internal/ca/ca.crt.pem 'https://pm-system-base.dpa.svc.cluster.local:30081/v1/internal/system/esn'" | tr -d '\r' | awk -F"[,:}]" '{for(i=1;i<=NF;i++){if($i~/'esn'\042/){print $(i+1)}}}' | tr -d '"' | sed -n 1p)
  if [ -z "${result}" ]; then
    log_error "Get esn failed!"
    clean_recovery_dir
    exit 1
  fi
  default_sftp_path=$(transfer_special_characters_for_expect_send "/var/sysbackup/${result}" | tr -d '\n\r')
}

main()
{
  prepare_for_recovery

  if [ "$#" -eq 0 ]; then
    get_local_backup
  elif [ "$#" -ge 2 ] && [ "$#" -le 4 ]; then
    sftp_user=$(transfer_special_characters_for_expect_send "$1")
    if ! check_ipv4 "$2"; then
      if ! check_ipV6 "$2"; then
        log_error "The SFTP host IP address ${2} is invalid!"
        clean_recovery_dir
        exit 1
        fi
    fi
    sftp_host="$2"
    log_info "The SFTP host IP address ${2} checked pass."
    if [ -z "$3" ]; then
      gen_default_sftp_path
      sftp_path="$default_sftp_path"
    else
      sftp_path=$(transfer_special_characters_for_expect_send "$3")
    fi
    sftp_copy_name=$(transfer_special_characters_for_expect_send "$4")
    read -rsp "Enter the sftp user's password: " sftp_password
    echo -e "\r"
    sftp_password=$(transfer_special_characters_for_expect_send "$sftp_password")
    download_backup_archived
  else
    log_error "Parameters error!"
    clean_recovery_dir
    exit 1
  fi

  check_recovery_file

  unpack_all_recovery_file

  decrypt_all_file

  unzip_sub_system_file

  invoke_sub_system_recovery

  reboot_pm_system

  clean_recovery_dir

  log_info "System management data recover success."
}

main "$@"