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
author:
  - "huanghaoshu"
'''

EXAMPLES = '''
# Example from Ansible Playbooks
- name: install agent
  oceanprotect.ansible.agent.install_agent:
    manager_hostname: "8.40.99.110"
    manager_username: "sysadmin"
    manager_password: "Huawei@123"
    agent_ips: "192.168.103.79,192.168.103.81"
    agent_username: "root"
    agent_password: "Huawei@12az"
    agent_osType: "LINUX"
    agent_ipType: "IPV4"
    agent_type: "REMOTE_AGENT"
    agent_macs: "safe"
  vars:
    ansible_python_interpreter: /usr/bin/python3
'''

RETURN = '''
megs:[{
        "agent_ip": "192.168.103.79",
        "agent_status": "安装成功"
        }]
'''

from ansible_collections.oceanprotect.ansible.plugins.module_utils.OceanProtectManager import OceanProtectManager
from ansible_collections.oceanprotect.ansible.plugins.module_utils.log.log import logger

object_type = {"VM": "vim.VirtualMachine"}


def main():
    module_args = dict(
        manager_hostname=dict(required=True, type='str'),
        manager_username=dict(required=True, type='str'),
        manager_password=dict(required=True, type='str', no_log=True),
        copy_name=dict(required=True, type='str'),
        protection_object_type=dict(required=True, type='str'),
        new_object_name=dict(required=True, type='str'),
    )

    module = OceanProtectManager(
        argument_spec=module_args,
        supports_check_mode=True
    )
    module_args = module.params
    try:
        status_code, res = module.PmManager.copiesShow(params={"page_size": 20,
                                                               "page_no": 0,
                                                               "orders": "-display_timestamp",
                                                               "conditions": {
                                                                   "resource_sub_type": ["vim.VirtualMachine",
                                                                                         "vim.HostSystem",
                                                                                         "vim.ClusterComputeResource"],
                                                                   "name": module_args["copy_name"]}})
        if status_code == 200 and res["total"] >= 1:
            for i in res["items"]:
                if i["name"] == module_args["copy_name"]:
                    copy_info = i
            send_params = {
                "copy_id": copy_info["uuid"],
                "object_type": object_type[module_args["protection_object_type"]],
                "restore_location": "O",
                "filters": [],
                "restore_objects": [],
                "restore_type": "CR",
                "target": {
                    "details": [],
                    "env_id": "",
                    "env_type": "Host",
                    "restore_target": copy_info["resource_location"]
                },
                "source": {
                    "source_location": copy_info["resource_location"],
                    "source_name": copy_info["resource_name"]
                },
                "ext_parameters": {
                    "restore_op_type": 0,
                    "vm_name": module_args["new_object_name"],
                    "power_on": "true",
                    "startup_network_adaptor": False,
                    "isForceNBDSsl": True,
                    "host_list": "[]",
                    "isDeleteOriginalVM": "false"
                }
            }
            status_code, res = module.PmManager.restoresPost(params=send_params)
            flag = True
            if status_code == 200:
                job_id = res[0]
                logger.info("restore task start success")
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
        else:
            logger.error("copy not found")
            module.fail_json(msg="copy not found", changed=False)
    except Exception as e:
        module.fail_json(msg=str(e), changed=False)


if __name__ == '__main__':
    main()
