#!/usr/bin/python3
# -*- coding: utf-8 -*-
import time

DOCUMENTATION = '''
---
module: oceanprotect.ansible.vmware.register.register_vmware
short_description: register resource
version_added: "1.6.0"
description:
  - 注册应用，支持vmware
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
  vmware_name:
       description:
      - vcenter name
    required: true 
  vmware_port:
    description:
      - vcenter port
    required: true 
  vmware_username:
    description:
      - vcenter username
    required: true 
  vmware_password:
    description:
      - vcenter password
    required: true 
  vmware_endpoint:
    description:
      - vcenter ip info
    required: true 
  vmware_storages_username:
    description:
      - storages username ;If the data storage type of the VM disk contains eVol storage, add storage resources. 
      Otherwise, the backup task will fail.
    required: false  
  vmware_storages_password:
    description:
      - storages username ;If the data storage type of the VM disk contains eVol storage, add storage resources. 
      Otherwise, the backup task will fail.
    required: false 
  vmware_storages_port:
    description:
      - storages port ;If the data storage type of the VM disk contains eVol storage, add storage resources. 
      Otherwise, the backup task will fail.
    required: false 
  vmware_storages_ip:
    description:
      - storages ip info ;If the data storage type of the VM disk contains eVol storage, add storage resources.
       Otherwise, the backup task will fail.
    required: false 
  
author:
  - "huanghaoshu"
'''

EXAMPLES = '''
# Example from Ansible Playbooks
- name: register vmware
  oceanprotect.ansible.vmware.register.register_vmware:
    manager_hostname: "8.40.98.43"
    manager_username: "sysadmin"
    manager_password: "Huawei@123"
    vmware_name: "vcenter01"
    vmware_port: 443
    vmware_username: "administrator@vsphere.local"
    vmware_password: "Huawei@123"
    vmware_endpoint: "8.40.97.103"
    vmware_storage_username: "admin"
    vmware_storage_password: "Admin@storage1"
    vmware_storage_port: 8088
    vmware_storage_ip: "8.40.97.149"
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
        vmware_name=dict(required=True, type='str'),
        vmware_port=dict(required=True, type=int),
        vmware_username=dict(required=True, type='str'),
        vmware_password=dict(required=True, type='str', no_log=True),
        vmware_endpoint=dict(required=True, type='str'),
        vmware_storage_username=dict(required=False, type='str'),
        vmware_storage_password=dict(required=False, type='str', no_log=True),
        vmware_storage_port=dict(required=False, type=int),
        vmware_storage_ip=dict(required=False, type='str'),
    )

    module = OceanProtectManager(
        argument_spec=module_args,
        supports_check_mode=True
    )
    register_info = ""
    module_args = module.params

    try:
        status_code, res = module.PmManager.environmentsGet(params={"page_size": 100, "page_no": 0,
                                                                    "conditions": {"type": "vSphere"}})
        flag = False
        if status_code == 200:
            for i in res["items"]:
                if i.get("endpoint") == module_args["vmware_endpoint"]:
                    register_info = module_args["vmware_endpoint"] + ":vmware have been register"
                    logger.info(register_info)
                    flag = True
                    break

        if not flag:
            send_params = {
                "name": module_args["vmware_name"],
                "user_name": module_args["vmware_username"],
                "password": module_args["vmware_password"],
                "port": module_args["vmware_port"],
                "endpoint": module_args["vmware_endpoint"],
                "rescan_interval_in_sec": 3600,
                "sub_type": "VMware",
                "type": "vSphere",
                "extend_context": {
                    "certification": "",
                    "cert_name": "",
                    "cert_size": "",
                    "revocation_list": "",
                    "crl_name": "",
                    "crl_size": "",
                    "tls_compatible": False,
                    "storages": []
                },
                "verify_cert": 0
            }
            # if module_args.get("vmware_storage_username"):
            #     send_params["storage_username"] = module_args["vmware_storage_username"]
            #     send_params["storage_password"] = module_args["vmware_storages_password"]
            #     send_params["storage_port"] = module_args["vmware_storages_port"]
            #     send_params["storage_ip"] = module_args["vmware_storages_ip"]
            logger.info(send_params)
            status_code, res = module.PmManager.environmentsPost(params=send_params)
            if status_code == 200:
                logger.info("send register task success")
            for i in range(10):
                status_code, res = module.PmManager.environmentsGet(params={"page_size": 100, "page_no": 0,
                                                                            "conditions": {"type": "vSphere"}})
                for i in res["items"]:
                    if i.get("endpoint") == module_args["vmware_endpoint"]:
                        register_info = module_args["vmware_endpoint"] + ":vmware register successs"
                        logger.info(register_info)
                        flag = True
                        break
                if flag:
                    break
                time.sleep(60)
            else:
                register_info = "vmware: %s register  fail" % module_args["vmware_endpoint"]
                logger.info(register_info)
    except Exception as e:
        module.fail_json(msg=e, changed=False)
    module.result['failed'] = False
    module.result['changed'] = False
    module.result['stdout'] = str(register_info)
    module.exit_json(**module.result)


if __name__ == '__main__':
    main()
