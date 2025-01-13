#!/usr/bin/python3
# -*- coding: utf-8 -*-
import datetime
import time

DOCUMENTATION = '''
---
module: oceanprotect.ansible.fileset.backup.backup 
short_description: start backup
version_added: "1.6.0"
description:
  - 文件集备份
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
  copy_name:
     description:
      - 备份副本名称
    最小长度:2
    最大长度:64
    required: true
  backup_type:
     description:
      - 备份类型 full/incremental
    required: true
  fileset_name:
     description:
      - 文件集名称
    required: true
author:
  - "huanghaoshu"
'''

EXAMPLES = '''
# Example from Ansible Playbooks
- name: backup
  oceanprotect.ansible.fileset.backup.backup:
    manager_hostname: "8.40.99.110"
    manager_username: "sysadmin"
    manager_password: "Huawei@123"
    copy_name: "test0002"
    backup_type: "full"
    fileset_name: "test002"
'''

RETURN = '''
megs:[]
'''

from ansible_collections.oceanprotect.ansible.plugins.module_utils.OceanProtectManager import OceanProtectManager
from ansible_collections.oceanprotect.ansible.plugins.module_utils.log.log import logger


def main():
    module_args = dict(
        manager_hostname=dict(required=True, type='str'),
        manager_username=dict(required=True, type='str'),
        manager_password=dict(required=True, type='str', no_log=True),
        copy_name=dict(required=True, type='str'),
        fileset_name=dict(required=True, type='str'),
        backup_type=dict(required=True, type='str'),
    )

    module = OceanProtectManager(
        argument_spec=module_args,
        supports_check_mode=True
    )
    module_args = module.params
    try:
        resource_id = ""
        sla_id = ""
        status_code, res = module.PmManager.resourceShow(params={"page_size": 20, "page_no": 0,
                                                                 "conditions": {
                                                                     "subType": ["Fileset"],
                                                                     "name": [["~~"],
                                                                              module_args["fileset_name"]]}})
        if status_code == 200 and len(res["records"]) > 0:
            for i in res["records"]:
                if i.get("name") == module_args["fileset_name"]:
                    resource_id = i.get("uuid")
                    if i.get("protectedObject"):
                        sla_id = i.get("protectedObject").get("slaId")
                    break

        send_params = {
            "copy_name":  module_args["copy_name"],
            "action": module_args["backup_type"],
            "sla_id": ""
        }
        if not (resource_id and sla_id):
            logger.error("backup object not found")
            module.fail_json(msg="backup object not found", changed=False)

        send_params["resource_id"] = resource_id
        send_params["sla_id"] = sla_id
        logger.info(send_params)
        status_code, res = module.PmManager.protectedObjectsResourceIdActionBackupPost(params=send_params)
        flag = True
        if status_code == 200:
            job_id = res[0]
            logger.info("backup task start success")
            start_date = datetime.datetime.now()
            while flag:
                status_code, res = module.PmManager.jobsGet(params={"jobId": job_id})
                if status_code == 200 and res["records"][0].get("status") == "RUNNING":
                    logger.info("backup task is running")
                    time.sleep(60)
                elif status_code == 200 and res["records"][0].get("status") == "SUCCESS":
                    flag = False
                    logger.info("backup task is success")
                elif status_code == 200:
                    logger.info("backup task is %s" % res["records"][0].get("status"))
                    time.sleep(60)
                now_date = datetime.datetime.now()
                if (now_date - start_date).seconds > 3600:
                    logger.error("backup task is exec 3600s please check")
                    break
        if flag:
            module.fail_json(msg="start backup fail", changed=False)
        else:
            module.exit_json(msg="backup success", changed=False)
    except Exception as e:
        module.fail_json(msg=str(e), changed=False)


if __name__ == '__main__':
    main()
