#!/usr/bin/python3
# -*- coding: utf-8 -*-
import datetime
import time

DOCUMENTATION = '''
---
module: install agent
short_description: install agent 
version_added: "1.6.0"
description:
  - 从备份软件上，推送代理主机安装包到 代理主机 安装 检查代理主机安装状态
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
    required: true

  restore_type:
     description:
      - 恢复类型
    required: true
  source_endpoint:
     description:
      - 原位置ip
    required: true
  target_endpoint:
     description:
      - 目标位置ip
    required: true
author:
  - "huanghaoshu"
'''

EXAMPLES = '''
# Example from Ansible Playbooks
- name: start restore
  oceanprotect.ansible.fileset.restore.restore:
    manager_hostname: "8.40.99.110"
    manager_username: "sysadmin"
    manager_password: "Huawei@123"
    copy_name: "test0002"
    restore_type: "normalRestore"
    source_endpoint: "192.168.103.99"
    target_endpoint: "192.168.103.98"
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
        restore_type=dict(required=True, type='str'),
        source_endpoint=dict(required=True, type='str'),
        target_endpoint=dict(required=False, type='str'),
    )

    module = OceanProtectManager(
        argument_spec=module_args,
        supports_check_mode=True
    )
    module_args = module.params
    try:
        copy_id = target_uuid = ""
        status_code, res = module.PmManager.copiesShow(params={"page_size": 20,
                                                               "page_no": 0,
                                                               "orders": "-display_timestamp",
                                                               "conditions":
                                                                   {"resource_sub_type": ["Fileset"],
                                                                    "name": module_args["copy_name"]}}
                                                       )
        if status_code == 200 and res["total"] >= 1:
            for i in res["items"]:
                if i["name"] == module_args["copy_name"]:
                    copy_id = i.get("uuid")
        status_code, res = module.PmManager.resourceShow(params={"page_size": 100, "page_no": 0,
                                                                 "conditions": {"subType": ["FilesetPlugin"]}})
        if status_code == 200:
            for i in res["records"]:
                if i["environment"].get("endpoint") == module_args["source_endpoint"]:
                    target_uuid = i["environment"]["uuid"]
        if not (copy_id and target_uuid):
            module.fail_json(msg="copy not found", changed=False)
        else:
            send_params = {
                "copyId": copy_id,
                "targetEnv": target_uuid,
                "restoreType": module_args["restore_type"],
                "targetLocation": "original",
                "filters": [],
                "agents": [],
                "extendInfo": {
                    "restoreOption": "OVERWRITING"
                },
                "scripts": {
                    "preScript": "",
                    "postScript": "",
                    "failPostScript": ""
                }
            }
            logger.info(send_params)
            status_code, res = module.PmManager.restoresPost_V2(params=send_params)
            flag = True
            if status_code == 200:
                job_id = res.get("uuid")
                logger.info("restore task start success jobid is %s" % job_id)
            time.sleep(60)
            start_date = datetime.datetime.now()
            while flag:
                status_code, res = module.PmManager.jobsGet(params={"jobId": job_id})
                if status_code == 200 and res["records"][0].get("status") == "RUNNING":
                    logger.info("restore task is running")
                    time.sleep(60)
                elif status_code == 200 and res["records"][0].get("status") == "SUCCESS":
                    flag = False
                    logger.info("restore task is success")
                elif status_code == 200:
                    logger.info("restore task is %s" % res["records"][0].get("status"))
                    time.sleep(60)
                now_date = datetime.datetime.now()
                if (now_date - start_date).seconds > 3600:
                    logger.error("restore task is exec 3600s please check")
                    break
            if flag:
                module.fail_json(msg="start restore fail", changed=False)
            else:
                module.exit_json(msg="restore success", changed=False)
    except Exception as e:
        module.fail_json(msg=str(e), changed=False)


if __name__ == '__main__':
    main()
