#!/usr/bin/python3
# -*- coding: utf-8 -*-
import time

DOCUMENTATION = '''
---
module: oceanprotect.ansible.archive.register_archive
short_description: register archive
version_added: "1.6.0"
description:
  - 注册归档存储
options:
  manager_hostname:
    description:
      - OceanProtectManager web hostname
    required: true
  manager_username:
    description:
      - OceanProtectManager web hostname
    required: true 
  manager_passwrod:
     description:
      - OceanProtectManager web hostname
    required: true
author:
  - "huanghaoshu"
'''

EXAMPLES = '''
# Example from Ansible Playbooks
- name: install agent
  oceanprotect.ansible.archive.register_archive:
    manager_hostname: "8.40.99.110"
    manager_username: "sysadmin"
    manager_password: "Huawei@123"
    archive_storage_name: "ArchiveStorage"
    archive_bucket_name: "test1"
    archive_index_bucket_name: "test2"
    archive_use_https: False
    archive_type: 2
    archive_cloud_type: 1
    archive_endpoint: "8.42.99.191"
    archive_ak: "C5E00C2F673B0E09BF67"
    archive_sk: "+m51ne/G2Y3XiNkvTLeV9tRI8KwAAAGMZzsOEu19"
    archive_proxy_enable: False
    archive_proxy_host_name: ""
    archive_proxy_username: ""
    archive_proxy_password: ""
    archive_alarm_enable: False
    archive_alarm_limit_value_unit: ""
    archive_alarm_threshold: 0
  vars:
    ansible_python_interpreter: /usr/bin/python3
'''

RETURN = '''
megs:[]
'''

from ansible_collections.oceanprotect.ansible.plugins.module_utils.OceanProtectManager import OceanProtectManager
from ansible_collections.oceanprotect.ansible.plugins.module_utils.log.log import logger

class CloudType(object):
    Pacific = '0'
    Obs = '1'
    S9000 = '2'
    AWS = '3'

def main():
    module_args = dict(
        manager_hostname=dict(required=True, type='str'),
        manager_username=dict(required=True, type='str'),
        manager_password=dict(required=True, type='str', no_log=True),
        archive_storage_name=dict(required=True, type='str'),
        archive_bucket_name=dict(required=True, type='str'),
        archive_index_bucket_name=dict(required=True, type='str'),
        archive_type=dict(required=False, type='str'),
        archive_cloud_type=dict(required=False, type='str'),
        archive_endpoint=dict(required=False, type='str'),
        archive_use_https=dict(required=False, type='str'),
        archive_ak=dict(required=False, type='str'),
        archive_sk=dict(required=False, type='str'),


        archive_proxy_enable=dict(required=True, type='str'),
        archive_proxy_host_name=dict(required=True, type='str'),
        archive_proxy_username=dict(required=True, type='str'),
        archive_proxy_password=dict(required=True, type='str'),

        archive_alarm_enable=dict(required=True, type='str'),
        archive_alarm_limit_value_unit=dict(required=True, type='str', no_log=True), # 1:TB 2:GB
        archive_alarm_threshold=dict(required=True, type='str', no_log=True),
    )

    module = OceanProtectManager(
        argument_spec=module_args,
        supports_check_mode=True
    )
    register_info = ""
    module_args = module.params
    try:
        status_code, res = module.PmManager.storagesGet(params={"endpoint": module_args["archive_endpoint"]})
        if status_code == 200 and len(res["records"]) > 0 :
            register_info = module_args["archive_endpoint"] + ":archive have been register"
            logger.info(register_info)
        else:
            send_params = {
                "storageName": module_args["archive_storage_name"],
                "type": module_args["archive_type"],
                "cloudType": module_args["archive_cloud_type"],
                "endpoint": module_args["archive_endpoint"],
                "useHttps": module_args["archive_use_https"],
                "bucketName": module_args["archive_bucket_name"],
                "ak": module_args["archive_ak"],
                "sk": module_args["archive_sk"],
                "indexBucketName": module_args["archive_index_bucket_name"],
                "proxyEnable": module_args["archive_proxy_enable"],
                "proxyHostName": module_args["archive_proxy_host_name"],
                "proxyUserName": module_args["archive_proxy_username"],
                "proxyUserPwd": module_args["archive_proxy_password"],
                "alarmEnable": module_args["archive_alarm_enable"],
                "alarmLimitValueUnit": module_args["archive_alarm_limit_value_unit"],
                "alarmThreshold": module_args["archive_alarm_threshold"],
                "alarmThresholdType": module_args["archive_alarm_threshold_type"],
            }

            status_code, res = module.PmManager.storagesPost(params=send_params)
            if status_code == 200:
                logger.info("send register task success")
            for i in range(10):
                status_code, res = module.PmManager.storagesGet(params={"endpoint": module_args["archive_endpoint"]})
                logger.info("Waiting for archive %s online" % module_args["archive_endpoint"])
                if len(res["records"]) > 0:
                    register_info = "archive: %s register  success" % module_args["archive_endpoint"]
                    logger.info(register_info)
                    break
                time.sleep(60)
            else:
                register_info = "archive: %s register  fail" % module_args["archive_endpoint"]
                logger.info(register_info)
    except Exception as e:
        module.fail_json(msg=e, changed=False)
    module.result['failed'] = False
    module.result['changed'] = False
    module.result['stdout'] = str(register_info)
    module.exit_json(**module.result)


if __name__ == '__main__':
    main()
