#!/usr/bin/python3
# -*- coding: utf-8 -*-
import time

DOCUMENTATION = '''
---
module: install agent
short_description: 创建保护 
version_added: "1.6.0"
description:
  - 
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
  sla_name:
     description:
      -  sla策略名称
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
- name: create protection
  oceanprotect.ansible.fileset.protection.create_protection:
    manager_hostname: "8.40.99.110"
    manager_username: "sysadmin"
    manager_password: "Huawei@123"
    sla_name: "test002"
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
        fileset_name=dict(required=True, type='str'),
        sla_name=dict(required=True, type='str'),

    )

    module = OceanProtectManager(
        argument_spec=module_args,
        supports_check_mode=True
    )
    module_args = module.params
    try:
        resource_id = ""
        status_code, res = module.PmManager.resourceShow(params={"page_size": 20, "page_no": 0,
                                                                 "conditions": {
                                                                     "subType": ["Fileset"],
                                                                     "name": [["~~"],
                                                                              module_args["fileset_name"]]}})
        if status_code == 200 and len(res["records"]) > 0:
            for i in res["records"]:
                if i.get("name") == module_args["fileset_name"]:
                    if i.get("protectedObject") and i.get("protectedObject").get("slaId"):
                        logger.info("fileset have protection")
                        module.exit_json(msg="fileset have protection", changed=False)
                    resource_id = i.get("uuid")
                    break

        if not resource_id:
            logger.error("have no fileset resource")
            module.fail_json(msg="have no fileset resource", changed=False)

        send_params = {
            "name": module_args["fileset_name"],
            "resources": [
                {
                    "resource_id": resource_id
                }
            ],
            "sla_id": "",
            "ext_parameters": {
                "channels": 10,
                "consistent_backup": True,
                "cross_file_system": True,
                "backup_nfs": True,
                "backup_smb": False,
                "sparse_file_detection": False,
                "backup_continue_with_files_backup_failed": True,
                "small_file_aggregation": False
            }
        }
        sla_id = ""
        status_code, res = module.PmManager.slasGet(params={"name": module_args["sla_name"]})
        if status_code == 200:
            for i in res["items"]:
                if i.get("name") == module_args["sla_name"]:
                    sla_id = i.get("uuid")
                    break
        if not sla_id:
            module.fail_json(msg="sla not create", changed=False)
        else:
            send_params["sla_id"] = sla_id

        status_code, res = module.PmManager.protectedObjectsBatchPost(params=send_params)
        if status_code == 200:
            logger.info("protection create success")
            module.exit_json(msg="protection create success")
        else:
            module.fail_json(msg="protection create fail", changed=False)
    except Exception as e:
        module.fail_json(msg=e, changed=False)




if __name__ == '__main__':
    main()
