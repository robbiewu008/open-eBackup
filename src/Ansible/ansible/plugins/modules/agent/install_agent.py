#!/usr/bin/python3
# -*- coding: utf-8 -*-
import time

DOCUMENTATION = '''
---
module: oceanprotect.ansible.agent.install_agent
short_description: 客户端代理自动注册
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
  agent_ips:
     description:
      - IPV4(支持网段或以逗号分隔):192.168.100.10-192.168.100.20,192.167.0.1，IPV6（只支持逗号分隔）:fe80::20c:29ff:fecb:d50a,fe80::20c:2
    required: true
  agent_username:
     description:
      - 远程连接用户名
        最小长度:1
        最大长度:255
    required: true
  agent_password:
     description:
      - 远程连接用户密码
        最小长度:1
        最大长度:255
    required: true
  agent_osType:
     description:
      - 操作系统类型 Windows/Linux/Unix/Others
        枚举值:
        	WINDOWS
        	LINUX
        	UNIX
        	OTHERS
    required: true
  agent_ipType:
    description:
      - 类型 IPV4,IPV6
        枚举值:
        	IPV4
        	IPV6
    required: true
  agent_type:
    description:
      - 主机Oracle/主机通用/外置VM/外置通用
        枚举值:
        	HOST_AGENT_ORACLE
        	HOST_AGENT
        	REMOTE_AGENT_VMWARE
        	REMOTE_AGENT
    required: true
  agent_macs:
     description:
      - 代理主机安全配置
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
        agent_ips=dict(required=True, type='str'),
        agent_username=dict(required=True, type='str'),
        agent_password=dict(required=True, type='str', no_log=True),
        agent_osType=dict(required=True, type='str'),
        agent_ipType=dict(required=True, type='str'),
        agent_type=dict(required=True, type='str'),
        agent_macs=dict(required=True, type='str'),
    )

    module = OceanProtectManager(
        argument_spec=module_args,
        supports_check_mode=True
    )
    install_info = ""
    module_args = module.params
    try:
        check_install_list = []
        install_success, install_fall = [], []
        agent_ips = module_args["agent_ips"] if module_args.get("agent_ips") else ""
        for ip in agent_ips.split(","):
            tmp = {'ips': ip,
                   'osType': module_args.get("agent_osType"),
                   'password': module_args.get("agent_password"),
                   'username': module_args.get("agent_username"),
                   'type': module_args.get("agent_type"),
                   'ipType': module_args.get("agent_ipType"),
                   'macs': module_args.get("agent_macs", "compatible")
                   }
            status_code, res = module.PmManager.hostAgentGet(params={"endpoint": tmp["ips"]})

            if status_code == 200 and len(res["records"]) > 0 and res["records"][0]["linkStatus"] == "1":
                install_info += tmp["ips"] + ":agent_status is success"
                logger.info("agent %s installed" % tmp["ips"])
                install_success.append(tmp["ips"])
            else:
                logger.info("start install agent %s task" % tmp["ips"])
                status_code, res = module.PmManager.hostAgentRegisterPost(tmp)
                if status_code == 200:
                    logger.info("post install agent %s task success" % tmp["ips"])
                    check_install_list.append(tmp)
        for tmp in check_install_list:
            for i in range(10):
                status_code, res = module.PmManager.hostAgentGet(params={"endpoint": tmp["ips"]})
                logger.info("Waiting for agent %s online" % tmp["ips"])
                if len(res["records"]) > 0 and res["records"][0]["linkStatus"] == "1":
                    logger.info(tmp["ips"] + ":agent_status is online")
                    install_success.append(tmp["ips"])
                    break
                else:
                    logger.info(tmp["ips"] + ":agent_status is offline")
                time.sleep(60)
            else:
                logger.info("install agent %s fail" % tmp["ips"])
                install_fall.append(tmp["ips"])
        if not install_success:
            module.fail_json(msg="agents install fail", changed=False)
        module.exit_json(msg="success install agent %s ;fail install agent: %s" % (install_success, install_fall))
    except Exception as e:
        module.fail_json(msg=str(e), changed=False)


if __name__ == '__main__':
    main()
