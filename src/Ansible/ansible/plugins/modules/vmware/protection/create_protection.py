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
author:
  - "huanghaoshu"
'''

EXAMPLES = '''
# Example from Ansible Playbooks
- name: create protection
  oceanprotect.ansible.vmware.protection.create_protection:
    manager_hostname: "8.40.99.110"
    manager_username: "sysadmin"
    manager_password: "Huawei@123"
    sla_name: "test001"
    vmware_endpoint: "8.40.103.21"
    protection_object_name: "ctt_can_be_deleted_s1reyy"
    protection_object_type: "VM"
  vars:
    ansible_python_interpreter: /usr/bin/python3
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
        sla_name=dict(required=True, type='str'),
        vmware_endpoint=dict(required=True, type='str'),
        protection_object_name=dict(required=True, type='str'),
        protection_object_type=dict(required=True, type='str'),
    )

    module = OceanProtectManager(
        argument_spec=module_args,
        supports_check_mode=True
    )
    progress_info = ""
    module_args = module.params
    try:

        status_code, res = module.PmManager.virtualMachineShow(params={
            "page_size": 20,
            "page_no": 0,
            "conditions": {
                "path": module_args["vmware_endpoint"],
                "type": module_args["protection_object_type"],
                "name": module_args["protection_object_name"]}
        })
        flag = False
        resource_uuid = ""

        if status_code == 200:
            for i in res["items"]:
                if i.get("name") == module_args["protection_object_name"]:
                    if i.get("sla_id"):
                        register_info = module_args["protection_object_name"] + ": have been protection"
                        logger.info(register_info)
                        flag = True

                    resource_uuid = i.get("uuid")
                    break
        if not flag:
            send_params = {
                "akOperationTips": False,
                "name": module_args["protection_object_name"],
                "resources": [
                    {
                        "resource_id": resource_uuid,
                        "filters": [
                            {
                                "filter_by": "ID",
                                "type": "DISK",
                                "rule": "ALL",
                                "mode": "INCLUDE",
                                "values": [
                                    "*"
                                ]
                            }
                        ]
                    }
                ],
                "sla_id": "",
                "ext_parameters": {
                    "pre_script": "",
                    "post_script": "",
                    "host_list": []
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
            else:
                module.fail_json(msg="protection create fail", changed=False)


    except Exception as e:
        module.fail_json(msg=e, changed=False)
    module.result['failed'] = False
    module.result['changed'] = False
    module.result['stdout'] = str(progress_info)
    module.exit_json(**module.result)


if __name__ == '__main__':
    main()
