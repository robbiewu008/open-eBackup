#!/usr/bin/python3
# -*- coding: utf-8 -*-
import json
import time

DOCUMENTATION = '''
---
module: oceanprotect.ansible.fileset.register.register_fileset
short_description: register resource
version_added: "1.6.0"
description:
  - 注册应用
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
  fileset_endpoint:
     description:
      - 文件集使用的代理主机ip
    required: true
  fileset_name:
     description:
      - 文件集名称
    required: true
  fileset_paths:
     description:
      - 文件集文件路径
    required: true
  fileset_filters:
     description:
      - 文件集过滤规则
    required: true
  
author:
  - "huanghaoshu"
'''

EXAMPLES = '''
# Example from Ansible Playbooks
- name: register fileset
  oceanprotect.ansible.fileset.register.register_fileset:
    manager_hostname: "8.40.99.110"
    manager_username: "sysadmin"
    manager_password: "Huawei@123"
    fileset_endpoint: "192.168.103.99"
    fileset_name: "test002"
    fileset_paths:
      - "/home/install.sh"
      - "/home/common.sh"
    fileset_filters: "[]"
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
        fileset_endpoint=dict(required=True, type='str'),
        fileset_name=dict(required=True, type='str'),
        fileset_paths=dict(required=True, type=list),
        fileset_filters=dict(required=True, type="str"),
    )

    module = OceanProtectManager(
        argument_spec=module_args,
        supports_check_mode=True
    )
    module_args = module.params

    try:

        # 查询是否重复
        status_code, res = module.PmManager.resourceShow(params={"page_size": 20, "page_no": 0,
                                                                 "conditions": {
                                                                     "subType": ["Fileset"],
                                                                     "name": [["~~"],
                                                                              module_args["fileset_name"]]}})
        if status_code == 200 and len(res["records"]) > 0:
            for i in res["records"]:
                if i.get("name") == module_args["fileset_name"]:
                    module.exit_json(msg="%s is exist" % module_args["fileset_name"])
                    break
        else:
            uuid = ""
            # 获取uuid
            status_code, res = module.PmManager.resourceShow(params={"page_size": 100, "page_no": 0,
                                                                     "conditions": {"subType": ["FilesetPlugin"]}})
            if status_code == 200:
                for i in res["records"]:
                    if i["environment"].get("endpoint") == module_args["fileset_endpoint"]:
                        uuid = i.get("uuid")
                        break
            if not uuid:
                logger.error("have no fileset agent")
                module.fail_json(msg="have no fileset agent", changed=False)
            paths = []
            for p in module_args["fileset_paths"]:
                paths.append({"name": p})
            send_params = {
                "name": module_args["fileset_name"],
                "parentUuid": uuid,
                "type": "Fileset",
                "subType": "Fileset",
                "extendInfo": {
                    "paths": json.dumps(paths),
                    "filters": module_args["fileset_filters"]
                }
            }
            status_code, res = module.PmManager.resourcesPost(params=send_params)
            if status_code == 200:
                logger.info("send register task success")
                for i in range(10):
                    status_code, res = module.PmManager.resourceShow(params={"page_size": 20, "page_no": 0,
                                                                             "conditions": {
                                                                                 "subType": ["Fileset"],
                                                                                 "name": [["~~"],
                                                                                          module_args[
                                                                                              "fileset_name"]]}})
                    for i in res["records"]:
                        if i.get("name") == module_args["fileset_name"]:
                            register_info = module_args["fileset_name"] + ":fileset register success"
                            logger.info(register_info)
                            module.exit_json(msg=register_info)
                            break

            else:
                register_info = "fileset: %s register  fail" % module_args["fileset_name"]
                logger.info(register_info)
                module.fail_json(msg=register_info, changed=False)

    except Exception as e:
        module.fail_json(msg=e, changed=False)


if __name__ == '__main__':
    main()
