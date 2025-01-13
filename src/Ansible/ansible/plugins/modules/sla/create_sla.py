#!/usr/bin/python3
# -*- coding: utf-8 -*-
import time

DOCUMENTATION = '''
---
module: install sla
short_description: create sla
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
- name: install sla
  oceanprotect.ansible.sla.create_sla:
    manager_hostname: "8.40.98.43"
    manager_username: "sysadmin"
    manager_password: "Huawei@123"
    sla_name: "test001"
    sla_application: "vim.VirtualMachine"
    sla_policy_list:
        - policy_001: {'name': '全量01',
                                  'action': full,
                                  'type': "backup",
                                  "retention_type": 2,
                                  "retention_duration": 1,
                                  "duration_unit": "y",
                                  "trigger": 4,
                                  "window_start": "00:00:00",
                                  "window_end": "00:00:00",
                                  "days_of_year": "2024-04-16",
                                  "trigger_action": "year",
                                  }
    sla_type: 1

'''

RETURN = '''
megs:[]
'''

from ansible_collections.oceanprotect.ansible.plugins.module_utils.OceanProtectManager import OceanProtectManager
from ansible_collections.oceanprotect.ansible.plugins.module_utils.log.log import logger

# backup方式
backup_action_list = ['full', 'difference_increment', 'cumulative_increment', 'log', 'permanent_increment']
# SLA备份策略&行为
sla_policy_list = ['backup', 'replication', 'archiving', 'full', 'difference_increment']


def get_create_backup_sla_public_params(device, **kwargs):
    # 传参检查
    policy_params = {
        "uuid": "",
        "name": kwargs.get('name'),
        "type": kwargs.get('type', 'backup'),
        "action": kwargs.get('action', 'full'),
        "retention": {
            "retention_type": kwargs.get('retention_type', 2),
            "retention_duration": kwargs.get('retention_duration', 1),
            "duration_unit": kwargs.get('duration_unit', 'd')
        },
        "schedule": {
            "trigger": kwargs.get('trigger', 1),
            "window_start": kwargs.get('window_start', '00:00:00'),
            "window_end": kwargs.get('window_end', '00:00:00')
        },
        "ext_parameters": kwargs.get('ext_parameters')
    }

    if policy_params['schedule']['trigger'] == 1:
        # 周期性备份参数，默认周期性备份
        policy_params['schedule']['interval'] = kwargs.get("interval", 10)
        policy_params['schedule']['interval_unit'] = kwargs.get("interval_unit", "h")
        policy_params['schedule']['start_time'] = kwargs.get("start_time", "2022-07-19T00:00:00")
    elif policy_params['schedule']['trigger'] == 4:
        # 指定时间备份参数
        policy_params['schedule']['trigger_action'] = kwargs.get("trigger_action", 'week')
        if policy_params['schedule']['trigger_action'] == 'week':
            policy_params['schedule']['days_of_week'] = kwargs.get("days_of_week", ['mon'])
        elif policy_params['schedule']['trigger_action'] == 'month':
            policy_params['schedule']['days_of_month'] = kwargs.get("days_of_month", "1")
        elif policy_params['schedule']['trigger_action'] == 'year':
            policy_params['schedule']['days_of_year'] = kwargs.get("days_of_year", "2022-05-12")

    # 限速策略
    if kwargs.get('qos_id'):
        qos_id = kwargs.get('qos_id')
        policy_params['ext_parameters']['qos_id'] = qos_id
    if kwargs.get('qos_name'):
        qos_obj = device.find('RateLimitPolicy', criteria={'name': kwargs.get('qos_name')},
                              forceSync=True)
        if len(qos_obj) > 0:
            qos_id = qos_obj[0].getProperty('uuid')
            policy_params['ext_parameters']['qos_id'] = qos_id

    # 自动重试关闭
    if kwargs.get('auto_retry') is False:
        ext_parameters = policy_params['ext_parameters']
        del ext_parameters['auto_retry_times']
        del ext_parameters['auto_retry_wait_minutes']
    return policy_params


def get_create_archive_sla_public_params(device, **kwargs):
    policy_params = {
        "uuid": "",
        "name": kwargs.get('name'),
        "type": kwargs.get('type', "archiving"),
        "action": kwargs.get('action', "archiving"),
        "ext_parameters": kwargs.get('ext_parameters'),
        "retention": {
            "retention_type": kwargs.get('retention_type', 2),
            "retention_duration": kwargs.get('retention_duration', 34),
            "duration_unit": kwargs.get('duration_unit', 'd')
        },
        "schedule": {
            "trigger": kwargs.get('trigger', 2),
        }
    }
    # 如果没有指定ext_parameters，则使用通用的ext_parameters
    if policy_params.get('ext_parameters') is None:
        policy_params['ext_parameters'] = {
            "qos_id": "",
            "protocol": kwargs.get('protocol', 2),
            "storage_id": "",
            "archiving_scope": kwargs.get('archiving_scope', "latest"),
            "network_access": kwargs.get('network_access', True),
            "auto_retry": kwargs.get('auto_retry', True),
            "auto_index": kwargs.get('auto_index', False),
            "auto_retry_times": kwargs.get('auto_retry_times', 3),
            "archive_target_type": kwargs.get('archive_target_type', 1),
            "auto_retry_wait_minutes": kwargs.get('auto_retry_wait_minutes', 5),
            "alarm_after_failure": kwargs.get('alarm_after_failure', False)
        }
        if policy_params['ext_parameters']['protocol'] == 2:
            # 查询对象存储目标id

            res = device.storagesGet()

            storages_list = res[0]['stdout']['records']
            if res[0]['rc'] != 200:
                logger.error(f'批量查询对象存储失败')
            storage_id = ''
            for obj in storages_list:
                if obj.get('name') == kwargs.get('storage_name'):
                    storage_id = obj.get('repositoryId')
                    break
            if not storage_id:
                logger.error(f'未找到指定的对象存储')
            policy_params['ext_parameters']['storage_id'] = storage_id
        elif policy_params['ext_parameters']['protocol'] == 7:
            # 查询磁带存储介质集id

            res = device.tapeLibraryMediaSetsGet()

            storages_list = res[0]['stdout']['records']
            if res[0]['rc'] != 200:
                logger.error(f'批量查询磁带介质集失败')
            storage_id = ''
            for obj in storages_list:
                if obj.get('mediaSetName') == kwargs.get('storage_name'):
                    storage_id = obj.get('mediaSetId')
                    break
            if not storage_id:
                logger.error(f'未找到指定的磁带介质集')
            policy_params['ext_parameters']['storage_id'] = storage_id

    # 限速策略
    if kwargs.get('qos_id'):
        qos_id = kwargs.get('qos_id')
        policy_params['ext_parameters']['qos_id'] = qos_id

    # 归档规则&保留规则
    if kwargs.get('specified_scope'):
        del policy_params['ext_parameters']['archiving_scope']
        policy_params['retention'] = {
            "retention_type": 2
        }

        if kwargs.get('specified_scope') == 'default':
            specified_scope_list = [
                {
                    "copy_type": "year",
                    "generate_time_range": "12",
                    "retention_unit": 'y',
                    "retention_duration": 1
                },
                {
                    "copy_type": "month",
                    "generate_time_range": "first",
                    "retention_unit": 'MO',
                    "retention_duration": 1
                },
                {
                    "copy_type": "week",
                    "generate_time_range": "mon",
                    "retention_unit": 'w',
                    "retention_duration": 1
                }
            ]
        else:
            specified_scope_list = kwargs.get('specified_scope')

        policy_params['ext_parameters']['specified_scope'] = specified_scope_list

    if policy_params['ext_parameters']['protocol'] == 7:
        policy_params['retention']['retention_type'] = 1
        del policy_params['retention']['retention_duration']
        del policy_params['retention']['duration_unit']

    # 归档时间
    if kwargs.get('trigger') == 1:
        schedule = {
            "trigger": kwargs.get('trigger'),
            "interval": kwargs.get('interval', 1),
            "interval_unit": kwargs.get('interval_unit', "h"),
            "start_time": kwargs.get('start_time', "2022-08-30 21:32:20")
        }
        policy_params['schedule'] = schedule
    if kwargs.get('trigger') == 2:
        schedule = {
            "trigger": kwargs.get('trigger'),
        }
        policy_params['schedule'] = schedule
    if kwargs.get('trigger') == 3:
        schedule = {
            "trigger": kwargs.get('trigger'),
            "interval": kwargs.get('interval', 1),
        }
        policy_params['schedule'] = schedule

    # 自动重试关闭
    if kwargs.get('auto_retry') is False:
        ext_parameters = policy_params['ext_parameters']
        del ext_parameters['auto_retry_times']
        del ext_parameters['auto_retry_wait_minutes']

    return policy_params


def get_create_log_backup_sla_public_params(device, **kwargs):
    # 传参检查
    policy_params = {
        "uuid": "",
        "name": kwargs.get('name'),
        "type": kwargs.get('type', 'backup'),
        "action": kwargs.get('action', 'log'),
        "retention": {
            "retention_type": kwargs.get('retention_type', 2),
            "retention_duration": kwargs.get('retention_duration', 1),
            "duration_unit": kwargs.get('duration_unit', 'd')
        },
        "schedule": {
            "trigger": kwargs.get('trigger', 1),
            "interval": kwargs.get('interval', 5),
            "interval_unit": kwargs.get('interval_unit', 'h'),
            "start_time": kwargs.get('start_time', '2022-07-21T00:00:00'),
        },
        "ext_parameters": kwargs.get('ext_parameters')
    }
    # 限速策略
    if kwargs.get('qos_id'):
        qos_id = kwargs.get('qos_id')
        policy_params['ext_parameters']['qos_id'] = qos_id

    # 自动重试关闭
    if kwargs.get('auto_retry') is False:
        ext_parameters = policy_params['ext_parameters']
        del ext_parameters['auto_retry_times']
        del ext_parameters['auto_retry_wait_minutes']
    return policy_params


def main():
    module_args = dict(
        manager_hostname=dict(required=True, type='str'),
        manager_username=dict(required=True, type='str'),
        manager_password=dict(required=True, type='str', no_log=True),
        sla_name=dict(required=True, type='str'),
        sla_application=dict(required=True, type='str'),
        sla_policy_list=dict(required=True, type=list),
        sla_type=dict(required=True, type=int),
    )

    module = OceanProtectManager(
        argument_spec=module_args,
        supports_check_mode=True
    )
    install_info = ""
    module_args = module.params
    try:
        status_code, res = module.PmManager.slasGet(params={"name": module_args["sla_name"]})
        if status_code == 200 and len(res["items"]) > 0 and res["items"][0].get("application") == module_args[
            'sla_application']:
            logger.info("sla is exist")
        else:
            send_params = {
                "uuid": "",
                "name": module_args["sla_name"],
                "type": module_args["sla_type"],
                "application": module_args.get('sla_application')
            }
            policy_send_params = []
            for policy in module_args["sla_policy_list"]:
                logger.info(policy)
                for k, policy_value in policy.items():
                    logger.info(policy_value)
                    if policy_value.get('action') is None:
                        logger.error('传参异常，policy_list中必须要传入action %s' % policy_value)
                    if policy_value.get('action') in (
                            'full', 'permanent_increment', 'difference_increment', 'cumulative_increment', 'log'):
                        ext_parameters = {
                            "qos_id": "",
                            "source_deduplication": policy_value.get('source_deduplication', False),
                            "alarm_after_failure": policy_value.get('alarm_after_failure', True),
                            "auto_retry": policy_value.get('auto_retry', True),
                            "auto_retry_times": 3,
                            "auto_retry_wait_minutes": 5,
                            "storage_info": policy_value.get('storage_info', {}),
                        }
                        if "ensure_consistency_backup" in policy_value.keys():
                            ext_parameters["ensure_consistency_backup"] = policy_value['ensure_consistency_backup']
                        if "ensure_deleted_data" in policy_value.keys():
                            ext_parameters["ensure_deleted_data"] = policy_value['ensure_deleted_data']
                        if "ensure_specifies_transfer_mode" in policy_value.keys():
                            ext_parameters["ensure_specifies_transfer_mode"] = policy_value['ensure_specifies_transfer_mode']
                        if "ensure_storage_layer_backup" in policy_value.keys():
                            ext_parameters["ensure_storage_layer_backup"] = policy_value['ensure_storage_layer_backup']
                        if "fine_grained_restore" in policy_value.keys():
                            ext_parameters["fine_grained_restore"] = policy_value['fine_grained_restore']
                        if "specifies_transfer_mode" in policy_value.keys():
                            ext_parameters["specifies_transfer_mode"] = policy_value['specifies_transfer_mode']
                        if "storage_info" in policy_value:
                            ext_parameters["storage_info"] = policy_value.get("storage_info").copy()
                        if module_args.get('sla_application') == 'Common':
                            del policy['ext_parameters']["source_deduplication"]
                            del policy['ext_parameters']["qos_id"]
                            del policy['ext_parameters']["copy_verify"]
                        policy_value['ext_parameters'] = ext_parameters
                        if policy_value.get('action') == 'log':
                            policy_params = get_create_log_backup_sla_public_params(module.PmManager, **policy_value)
                        else:
                            policy_params = get_create_backup_sla_public_params(module.PmManager, **policy_value)
                        policy_send_params.append(policy_params)

                    if policy_value.get('action') == 'archiving':
                        policy_params = get_create_archive_sla_public_params(module.PmManager, **policy_value)
                        policy_send_params.append(policy_params)

            send_params['policy_list'] = policy_send_params
            logger.info(send_params)
            status_code, res = module.PmManager.slasPost(params=send_params)
            if status_code == 200:
                logger.info("send register task success")
            else:
                logger.info("send create sla task fail")
                module.fail_json(msg="send create sla task fail", changed=False)
            status_code, res = module.PmManager.slasGet(params={"name": module_args["sla_name"]})
            if status_code == 200:
                logger.info("send register task success")
            else:
                logger.info("create sla fail")
                module.fail_json(msg="create sla fail", changed=False)
    except Exception as e:
        module.fail_json(msg=str(e), changed=False)
    module.result['failed'] = False
    module.result['changed'] = False
    module.result['stdout'] = str(install_info)
    module.exit_json(**module.result)


if __name__ == '__main__':
    main()
