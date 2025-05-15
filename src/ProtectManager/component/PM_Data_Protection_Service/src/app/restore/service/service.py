# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
import json
import uuid
from http import HTTPStatus

from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient
from app.common.clients.resource_client import ResourceClient
from app.common.constants.constant import RestoreConstants
from app.common.context.context import Context
from app.common.enums.license_enum import FunctionEnum
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.schedule_enum import ScheduleTypes
from app.common.events.consumer import EsEvent
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.license import validate_license_by_resource_type
from app.common.rpc.system_base_rpc import encrypt
from app.restore.client.copy_client import CopyClient
from app.restore.client.restore_client import RestoreClient
from app.restore.schema.restore import RestoreLocation, RestoreRequestSchema, RestoreType

log = logger.get_logger(__name__)

NAME = "restore_manager"

VM_OBJECT = "vm"
FILESET_OBJECT = "fileset"
DB_OBJECT = "db"

CATALOG_RESPONSE_TOPIC = "Restore_CatalogResponse"
LOCK_RESPONSE_TOPIC = "Restore_LockResponse"
LOCK_TOPIC = "Restore_Lock"
ENCRYPT_RESOURCE_SUBTYPE_SET = {
    ResourceSubTypeEnum.VirtualMachine.value,
    ResourceSubTypeEnum.FusionCompute.value,
    ResourceSubTypeEnum.HCSCloudHost.value,
    ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.value,
    ResourceSubTypeEnum.CNWARE_VM.value,
    ResourceSubTypeEnum.NUTANIX_VM.value,
    ResourceSubTypeEnum.HYPER_V_VM.value,
    ResourceSubTypeEnum.APSARA_STACK_INSTANCE.value,
    ResourceSubTypeEnum.FUSION_ONE_COMPUTE.value
}


def submit_restore_job(restore_req: RestoreRequestSchema, user_id: str):
    check_sub_object_size(restore_req)
    restore_req.request_id = str(uuid.uuid4())
    task = RestoreClient.create_task(restore_req)
    log.info(f"create task success task={task}")
    # 根据不同资源类型，对恢复参数进行补充
    restore_req = modify_restore_req_by_resource_type(
        restore_req.object_type, restore_req)
    if task is None:
        raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR,
                                   message="crate task failed")
    task["userId"] = user_id
    task['data']['task_gui_request'] = json.dumps(restore_req.dict())
    task['data']['task_type'] = 'restore_v1'
    schedule = {
        "schedule_type": ScheduleTypes.immediate.value,
        "action": 'restore',
        "params": restore_req.json(),
        "context": True,
        "task": json.dumps(task)
    }
    response = SystemBaseHttpsClient().request(
        "POST", f'/v1/schedules', body=json.dumps(schedule))
    if response.status != HTTPStatus.OK:
        raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR,
                                   message="Submit restore failed")
    return [restore_req.request_id]


def check_sub_object_size(restore_req: RestoreRequestSchema):
    restore_objects = restore_req.restore_objects
    if len(restore_objects) == 0:
        return
    if len(restore_objects) > RestoreConstants.RESTORE_OBJECT_MAX_NUM_LIMIT:
        raise EmeiStorBizException(CommonErrorCodes.RESTORE_SUB_OBJECT_NUM_MAX_LIMIT,
                                   RestoreConstants.RESTORE_OBJECT_MAX_NUM_LIMIT,
                                   error_message="Total sub object num over 1000")
    size = 0
    for sub_object in restore_objects:
        size += len(sub_object.encode('UTF-8'))
    if size > RestoreConstants.RESTORE_OBJECT_MAX_BYTE:
        raise EmeiStorBizException(error=CommonErrorCodes.RESTORE_SUB_OBJECT_MAX_BYTES,
                                   error_message="Total sub object size over 1024 * 1000")


def modify_restore_req_by_resource_type(resource_type, restore_body):
    # vmware恢复需要往上下文中塞入parent_uuid也即是计算资源uuid 用于支持vmware恢复后续资源刷新
    if resource_type == ResourceSubTypeEnum.VirtualMachine.value:
        parent_uuid = get_vm_computer_res_id(restore_body)
        log.info(f"get vmware computer_res_uuid={parent_uuid}")
        if parent_uuid is not None:
            restore_body.ext_parameters["parentUuid"] = parent_uuid
    # 虚拟化文件级恢复对目标恢复对象密码进行加密
    if resource_type in ENCRYPT_RESOURCE_SUBTYPE_SET and "PASSWORD" in restore_body.ext_parameters:
        if restore_body.ext_parameters["PASSWORD"] is not None:
            restore_body.ext_parameters["PASSWORD"] = encrypt(restore_body.ext_parameters["PASSWORD"])
    return restore_body


def get_vm_computer_res_id(restore_req: RestoreRequestSchema):
    parent_uuid = None
    # 整机恢复原位置 新位置需要刷新资源
    if restore_req.restore_location == RestoreLocation.origin.value and \
            restore_req.ext_parameters.get("restore_op_type") == 0:
        # 原位置从副本里面取
        copy_info = CopyClient.query_copies(0, 1, {"uuid": restore_req.copy_id}).get("items")[0]
        resource_properties = json.loads(copy_info["resource_properties"])
        parent_uuid = resource_properties.get("parent_uuid")
    if restore_req.restore_location == RestoreLocation.new.value and \
            restore_req.ext_parameters.get("restore_op_type") == 0:
        # 新位置从target里取
        parent_uuid = restore_req.target.env_id
    # 要对parent_uuid的资源进行判断 如果是集群下的主机 要塞集群的uuid进去
    try:
        env_info = ResourceClient.query_resource(parent_uuid)
    except Exception as e:
        log.exception(f"error is {e.__class__.__name__}")
        return None
    finally:
        pass
    if env_info is None:
        return None
    elif env_info.get("sub_type") == ResourceSubTypeEnum.ClusterComputeResource.value:
        return parent_uuid
    elif env_info.get("sub_type") == ResourceSubTypeEnum.HostSystem.value:
        parent_info = ResourceClient.query_resource(env_info.get("parent_uuid"))
        if parent_info.get("sub_type") == ResourceSubTypeEnum.ClusterComputeResource.value:
            parent_uuid = parent_info.get("uuid")
    return parent_uuid


def restore_prepare(request: EsEvent, **_):
    request_id = request.request_id
    context = Context(request_id)

    target = context.get("target", dict)
    env_id = target['env_id']
    env_type = target['env_type']

    source = context.get("source", dict)
    db_name = source['source_name'] if source else ''

    copy_id = context.get("copy_id")
    lock_resource_ids = [{"id": copy_id, "lock_type": "r"}] if copy_id else []

    object_type = context.get("object_type")
    lock_list = (ResourceSubTypeEnum.DB2.value, ResourceSubTypeEnum.MySQL.value, ResourceSubTypeEnum.SQLServer.value)
    if object_type in lock_list:
        db_uuid = generate_resource_id(context, object_type, env_id, copy_id)
        log.info(f"db_uuid:{db_uuid}.")
        lock_resource_ids.append({"id": db_uuid, "lock_type": "w"})
    if object_type == "Oracle" and env_type == "Host":
        target_db_uuid = RestoreClient.get_target_database(env_id, db_name)
        if target_db_uuid:
            lock_resource_ids.append({"id": target_db_uuid, "lock_type": "w"})
    if object_type == ResourceSubTypeEnum.VirtualMachine.value:
        vm_resource_id = generate_vm_resource_id(context, copy_id)
        log.info(f"vm_resource_lock_id:{vm_resource_id}.")
        if vm_resource_id is not None:
            lock_resource_ids.append({"id": vm_resource_id, "lock_type": "w"})

    # 除 VMware 以外的虚拟化应用
    non_vmware_virtualization_list = [
        ResourceSubTypeEnum.NUTANIX_VM.value,
        ResourceSubTypeEnum.CNWARE_VM.value,
        ResourceSubTypeEnum.HYPER_V_VM.value,
        ResourceSubTypeEnum.FusionCompute.value,
        ResourceSubTypeEnum.FUSION_ONE_COMPUTE.value
    ]
    # 云平台
    cloud_platform_list = [
        ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.value,
        ResourceSubTypeEnum.APSARA_STACK_INSTANCE.value,
        ResourceSubTypeEnum.HCSCloudHost.value
    ]
    # 在文件级细粒度恢复的时，对除 VMware 以外的虚拟化应用和云平台的恢复目标位置资源加写锁
    if object_type in (non_vmware_virtualization_list + cloud_platform_list) and env_id is not None:
        lock_resource_ids.append({"id": env_id, "lock_type": "w"})

    return True, {"lock_resources": lock_resource_ids}


def restore_process(request: EsEvent):
    context = Context(request.request_id)
    object_type = context.get("object_type")
    restore_type = context.get("restore_type")
    function = {
        RestoreType.CR.value: None,
        RestoreType.IR.value: FunctionEnum.INSTANT_RECOVERY,
        RestoreType.FLR.value: FunctionEnum.FINE_GRAINED_RECOVERY,
    }.get(restore_type)
    if function is not None:
        validate_license_by_resource_type(function, object_type)
    return {"topic": "protection.restore", "message": {}}


def generate_vm_resource_id(context, copy_id):
    ext_parameter_dict = context.get("ext_parameters", dict)
    restore_location = context.get("restore_location")
    restore_type = context.get("restore_type")
    restore_op_type = ext_parameter_dict.get("restore_op_type")
    target = context.get("target", dict)
    copy_info = CopyClient.query_copies(0, 1, {"uuid": copy_id}).get("items")[0]
    resource_id = None
    if restore_type == 'CR' and restore_op_type == 1:
        # 磁盘恢复 无论新位置原位置 都会产生原机操作 资源id从目标里取
        resource_id = target.get("details")[0].get("target_id")
    elif restore_location == "O" and ext_parameter_dict.get("isDeleteOriginalVM") == "true":
        # CR IR 恢复 原位置覆盖原机 资源id从副本里面取
        resource_id = copy_info["resource_id"]
    elif restore_location == "O" and restore_type == "FLR":
        # FLR恢复 原位置覆盖原机 资源id从副本里面取
        resource_id = copy_info["resource_id"]
    return resource_id


def generate_resource_id(context, object_type, env_id, copy_id):
    ext_parameter_dict = context.get("ext_parameters", dict)
    copy_info = CopyClient.query_copies(0, 1, {"uuid": copy_id}).get("items")[0]
    host_id = env_id
    sub_type = object_type
    database_name = copy_info["resource_name"]
    resource_property = json.loads(copy_info["resource_properties"])
    instance_name = resource_property.get("instance_names")

    if ext_parameter_dict.get("EEE_RESTORE_INSTANCENAME") is not None and len(
            ext_parameter_dict.get("EEE_RESTORE_INSTANCENAME")) > 0:
        instance_name = ext_parameter_dict.get("EEE_RESTORE_INSTANCENAME")

    if ext_parameter_dict.get("EEE_RESTORE_DB2_NEW_NAME") is not None and len(
            ext_parameter_dict.get("EEE_RESTORE_DB2_NEW_NAME")) > 0:
        database_name = ext_parameter_dict.get("EEE_RESTORE_DB2_NEW_NAME")

    if ext_parameter_dict.get("EEE_MSSQL_RESTORE_DB_NEW_NAME") is not None and len(
            ext_parameter_dict.get("EEE_MSSQL_RESTORE_DB_NEW_NAME")) > 0:
        database_name = ext_parameter_dict.get("EEE_MSSQL_RESTORE_DB_NEW_NAME")

    if host_id is None or len(host_id) == 0:
        host_id = resource_property.get("root_uuid")
    context.set("host_id", host_id)
    context.set("sub_type", sub_type)
    context.set("database_name", database_name)
    context.set("instance_name", instance_name)
    return str(uuid.uuid5(uuid.NAMESPACE_X500, host_id + sub_type + instance_name + database_name))
