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
from datetime import datetime
from enum import Enum
from typing import List

from pydantic import Field, BaseModel, root_validator

from app.common.enums.anti_ransomware_enum import AntiRansomwareEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.copy_catalog.common.copy_status import CopyStatus
from app.common.enums.sla_enum import RetentionTimeUnit, RetentionTypeEnum, WormValidityTypeEnum
from app.common.schemas.common_schemas import BaseOrmModel


class CopyInfoSchema(BaseOrmModel):
    uuid: str = Field(None, description="副本ID")
    chain_id: str = Field(None, description="副本链ID")
    timestamp: str = Field(description="副本时间戳")
    display_timestamp: datetime = Field(description="副本时间戳")
    deletable: bool = Field(description="副本是否可删除")
    status: str = Field(description="副本有效性")  # 0 invalid; 1 valid
    location: str = Field(None, description="副本位置")  # 0 local; 1 remote; 2 cloud
    backup_type: int = Field(None, description="备份类型")  # 1：完全备份 2：增量备份 3：差异备份 4：日志备份 5: 永久增量备份
    generated_by: str = Field(
        description="副本生成类型")  # 0 Imported, 1 Replicated, 2 Backup, 3 Archive, 4 Live Mount
    gn: int = Field(None, description="gn")
    generated_time: datetime = Field(None, description="副本时间戳")
    features: int = Field(None, description="副本支持的特性")
    indexed: str = Field(description="副本建立索引状态")  # 0 unindexed, 1 indexed, 2 indexing
    generation: int = Field(description="副本代数")
    parent_copy_uuid: str = Field(None, description="父副本ID")
    retention_type: int = Field(description="副本保留类型：1（永久保留）2（指定时间保留）")
    retention_duration: int = Field(None, description="副本保留时间")
    duration_unit: str = Field(None, description="副本保留时间单位（天、周、月、年）")
    expiration_time: datetime = Field(None, description="副本过期时间")
    properties: str = Field(None, description="副本扩展属性")  # JSON
    resource_id: str = Field(description="资源ID")
    resource_name: str = Field(description="资源名称")
    resource_type: str = Field(description="资源类型")
    resource_sub_type: str = Field(None, description="资源子类型")
    resource_location: str = Field(description="资源位置")
    resource_status: str = Field(description="资源状态")
    resource_properties: str = Field(description="资源属性（JSON格式）")
    resource_environment_name: str = Field(None, description="资源环境名称")
    resource_environment_ip: str = Field(None, description="资源环境IP")
    sla_name: str = Field(None, description="SLA名称")
    sla_properties: str = Field(None, description="SLA属性（JSON格式）")
    user_id: str = Field(None, description="副本资源的用户id")
    is_archived: bool = Field(None, description="副本是否归档")
    is_replicated: bool = Field(None, description="副本是否复制")
    detail: str = Field(None, description="副本详情")
    name: str = Field(None, description="副本名字")
    storage_id: str = Field(None, description="存储库id")
    source_copy_type: int = Field(None, description="原始副本类型")  # 1：完全备份2：增量备份3：差异备份4：日志备份5:永久增量备份
    worm_status: int = Field(default=1, description="副本worm状态")
    device_esn: str = Field(None, description="副本所在集群")
    pool_id: str = Field(None, description="副本所在存储池id")
    origin_backup_id: str = Field(None, description="原备份副本id")
    origin_copy_time_stamp: datetime = Field(None, description="原备份副本时间戳")
    storage_unit_id: str = Field(None, description="存储单元id")
    storage_snapshot_flag: bool = Field(None, description="是否为存储快照")
    extend_type: str = Field(None, description="扩展类型")
    worm_validity_type: int = Field(None, description="worm有效期类型")  # worm有效期类型 1 同副本过期 2.自定义worm有效时间
    worm_retention_duration: int = Field(None, description="副本保留时间")  # worm保留时间
    worm_duration_unit: str = Field(None, description="副本保留时间单位（天、周、月、年）")  # worm保留时间单位
    worm_expiration_time: datetime = Field(None, description="worm副本过期时间")  # worm过期时间
    storage_unit_status: int = Field(None, description="存储单元状态")  # 副本所在的存储介质状态  备份 归档 复制 挂载
    browse_mounted: str = Field(None, description="虚拟化副本细粒度浏览挂载状态") # Umount, Mounted, Mounting

class CopyInfoWithArchiveFieldsSchema(CopyInfoSchema):
    storage_id: str = Field(None, description="存储ID")


class CopySchema(CopyInfoSchema):
    uuid: str = Field(description="副本ID")
    gn: int = Field(description="副本序列号")  # generate number
    prev_copy_id: str = Field(None, description="上一个副本的ID")
    next_copy_id: str = Field(None, description="下一个副本的ID")
    prev_copy_gn: int = Field(None, description="上一个副本的gn")
    next_copy_gn: int = Field(None, description="下一个副本的gn")
    device_esn: str = Field(None, description="设备esn")
    cluster_name: str = Field(None, description="集群名称")


class CopyInfoQuerySchema(CopySchema):
    storage_unit_name: str = Field(None, description="存储单元名称")


class CopyResourceSummarySchema(BaseModel):
    resource_id: str = Field(description="资源ID")
    resource_name: str = Field(description="资源名称")
    resource_type: str = Field(description="资源类型")
    resource_sub_type: str = Field(description="资源子类型")
    resource_location: str = Field(description="资源位置")
    resource_status: str = Field(description="资源状态")
    properties: str = Field(None, description="副本资源属性")
    resource_properties: str = Field(description="资源属性")
    resource_environment_name: str = Field(None, description="资源环境名称")
    resource_environment_ip: str = Field(None, description="资源环境IP")
    sla_name: str = Field(None, description="SLA名称")
    protected_resource_id: str = Field(None, description="保护资源ID")
    protected_object_uuid: str = Field(None, description="保护对象UUID")
    protected_sla_id: str = Field(None, description="保护SLA ID")
    protected_sla_name: str = Field(None, description="保护SLA名称")
    protected_status: bool = Field(None, description="保护状态")
    copy_count: int = Field(description="资源副本数量")


class CopyStatisticView(Enum):
    YEAR = "year"
    MONTH = "month"


class CopyStatistic(BaseModel):
    index: str = Field(description="索引值，对应：月/日")
    count: int = Field(description="副本统计结果")


class CopyRetentionPolicySchema(BaseModel):
    resource_id: str = Field(None, description="资源ID")
    worm_validity_type: WormValidityTypeEnum = Field(WormValidityTypeEnum.copy_retention_time_consistent,
                                                     description="WORM有效保留类型")
    retention_type: RetentionTypeEnum = Field(RetentionTypeEnum.permanent, description="保留策略类型")
    retention_duration: int = Field(0, description="副本保留周期")
    duration_unit: RetentionTimeUnit = Field(None, description="副本保留周期单位")


class CopyWormRetentionSettingSchema(BaseModel):
    convert_worm_switch: bool = Field(False, description="是否转换worm副本")
    worm_validity_type: WormValidityTypeEnum = Field(WormValidityTypeEnum.copy_retention_time_consistent,
                                                     description="WORM有效保留类型")
    retention_duration: int = Field(0, description="副本保留周期")
    duration_unit: RetentionTimeUnit = Field(None, description="副本保留周期单位")


class CopyStatusUpdate(BaseModel):
    status: CopyStatus = Field(description="副本状态")
    deletable: bool = Field(None, description="副本是否可删除")
    timestamp: str = Field(None, description="副本时间戳")
    display_timestamp: datetime = Field(None, description="副本时间戳")
    is_archived: bool = Field(None, description="是否归档")
    is_replicated: bool = Field(None, description="副本是否已被复制")
    expiration_time: datetime = Field(None, description="副本过期时间")
    generated_time: datetime = Field(None, description="副本时间戳")


class CopyRestoreInformation(BaseModel):
    PFILE: str = Field(None, description="目标副本对应数据库配置")


class CopyProtectionSchema(BaseModel):
    protected_resource_id: str = Field(description="保护资源ID")
    protected_object_uuid: str = Field(description="保护对象UUID")
    protected_sla_id: str = Field(description="保护SLA ID")
    protected_sla_name: str = Field(None, description="保护SLA名称")
    protected_status: bool = Field(description="保护状态")


class ProtectedCopyBatchOperationReq(BaseModel):
    resource_ids: List[str] = Field(..., description="批量操作资源id")


class CopyArchiveMapSchema(BaseModel):
    copy_id: str = Field(description="副本ID")
    storage_id: str = Field(description="存储ID")
    resource_id: str = Field(description="资源ID")


class ReplicatedCopiesSchema(BaseModel):
    copy_id: str = Field(description="副本ID")
    resource_id: str = Field(description="资源ID")
    esn: str = Field(description="esn")


class SaveImportCopyRequest(BaseModel):
    uuid: str = Field(description="导入副本ID")
    name: str = Field(description="DWS导入副本文件系统名")
    generated_time: int = Field(description="导入副本生成时间戳(如：1628604206)秒")
    generated_by: str = Field(description="副本生成方式")
    resource_type: str = Field(description="资源类型")
    resource_sub_type: str = Field(description="子资源类型")
    resource_id: str = Field(description="资源ID")
    chain_id: str = Field(description="ChainID")


class CopyAntiRansomwareReq(BaseModel):
    status: int = Field(description="副本检测状态")
    model: str = Field(None, description="副本模型版本")
    detection_start_time: str = Field(None, description="检测开始时间")
    detection_end_time: str = Field(None, description="检测结束时间")
    report: str = Field(None, description="副本报告")
    tenant_id: str = Field(None, max_length=64, description="租户id")
    tenant_name: str = Field(None, max_length=512, description=" 租户名称")
    total_file_size: int = Field(None, description="总文件大小，单位B")
    changed_file_count: int = Field(None, description="修改变化数量")
    added_file_count: int = Field(None, description="新增文件数量")
    deleted_file_count: int = Field(None, description="删除文件数量")
    infected_file_count: int = Field(None, description="异常文件数量")
    backup_software: str = Field(None, description="备份软件类型")
    generate_type: str = Field(None, description="快照生成类型")
    infected_file_detect_duration: int = Field(None, description="可疑文件检测耗时")

    @root_validator
    def valid_params(cls, values):
        status = values.get("status")

        if status in (AntiRansomwareEnum.UNINFECTING.value,
                      AntiRansomwareEnum.INFECTING.value) \
                and (
                values.get("model") is None or
                values.get("detection_start_time") is None or
                values.get("detection_end_time") is None or
                values.get("report") is None):
            raise EmeiStorBizException(error=CommonErrorCodes.ERR_PARAM,
                                       error_message="The parameter is incorrect.")
        return values


class ModifyCopyAntiRansomwareDetectionStatusReq(BaseModel):
    copy_id: str = Field(None, description="副本ID")
    resource_id: str = Field(None, description="资源ID")
    vstore_id: str = Field(None, description="租户ID")
    old_status: int = Field(description="旧检测状态")
    new_status: int = Field(description="更新的检测状态")
    rollback_worm_status: bool = Field(False, description="更新的检测状态")


class CopyAntiRansomwareReport(BaseModel):
    copy_id: str = Field(description="副本ID")
    timestamp: str = Field(None, description="副本时间戳")
    model: str = Field(None, description="防勒索检测模型")
    status: int = Field(description="检测状态")
    detection_duration: int = Field(None, description="检测持续时间")
    detection_time: str = Field(None, description="副本检测时间")
    report: str = Field(None, description="防勒索检测报告")
    infected_file_detect_duration: int = Field(None, description="可疑文件检测耗时")


class CopyAntiRansomware(CopyInfoSchema):
    model: str = Field(None, description="防勒索检测模型")
    anti_status: int = Field(None, description="检测状态")
    detection_time: str = Field(None, description="副本检测时间")
    snapshot_time: str = Field(None, description="快照时间")
    total_file_size: int = Field(None, description="总文件大小")
    changed_file_count: int = Field(None, description="修改文件数量")
    added_file_count: int = Field(None, description="新增文件数量")
    deleted_file_count: int = Field(None, description="删除文件数量")
    handle_false: bool = Field(None, description="误报处理")
    generate_type: str = Field(None, description="生成方式")
    is_security_snapshot: bool = Field(None, description="安全快照")
    tenant_name: str = Field(None, description="租户名称")
    file_sub_type: int = Field(None, description="文件系统类型")
    infected_file_count: int = Field(None, description="异常文件数量")
    backup_software: str = Field(None, description="备份软件")


class CopyAntiRansomwarFeedbackReq(BaseModel):
    resource_sub_type: str = Field(..., description="资源类型")
    name: str = Field(..., description="资源名称")
    copy_ids: list = Field(..., description="副本id列表")


class CopyAntiRansomwareStatistics(BaseModel):
    resource_id: str = Field(None, description="资源ID")
    name: str = Field(None, description="资源名称")
    location: str = Field(None, description="资源位置")
    resource_sub_type: str = Field(None, description="资源类型")
    policy_name: str = Field(None, description="策略名称")
    policy_id: int = Field(None, description="策略id")
    total_copy_num: int = Field(description="副本总数量")
    infected_copy_num: int = Field(None, description="感染副本数量")
    uninfected_copy_num: int = Field(None, description="未感染副本数量")
    detecting_copy_num: int = Field(None, description="正在检测副本数量")
    uninspected_copy_num: int = Field(None, description="未检测副本数量")
    prepare_copy_num: int = Field(None, description="准备中副本数量")
    abnormal_copy_num: int = Field(None, description="异常副本数量")
    tenant_name: str = Field(None, description="租户名称")
    tenant_id: str = Field(None, description="租户id")
    latest_detection_time: str = Field(None, description="最新检测时间")
    latest_snapshot_time: str = Field(None, description="最新快照时间")
    status: int = Field(None, description="检测安全状态")
    device_ip: str = Field(None, description="所属存储设备IP")
    device_name: str = Field(None, description="所属存储设备名称")
    total_file_size: int = Field(None, description="最新检测快照的总文件大小")
    added_file_count: int = Field(None, description="最新检测快照的新增文件数量")
    changed_file_count: int = Field(None, description="最新检测快照的修改文件数量")
    deleted_file_count: int = Field(None, description="最新检测快照的删除文件数量")
    latest_copy_id: str = Field(None, description="最新副本ID")
    total_detect_copy_num: int = Field(None, description="检测副本总数量")
    handle_false_count: int = Field(None, description="误报处理数量")
    end_copy_time: str = Field(None, description="快照结束时间")
    start_copy_time: str = Field(None, description="快照起始时间")
    file_sub_type: int = Field(None, description="文件系统类型")
    infected_file_count: int = Field(None, description="最新检测快照的异常文件数量")
    backup_software: str = Field(None, description="最新检测快照的备份软件")
    generate_type: str = Field(None, description="最新检测快照的生成类型")


class CopyAntiRansomwareSummary(BaseModel):
    resource_sub_type: str = Field(None, description="资源类型")
    detection_date: str = Field(None, description="检测日期")
    total_copy_num: int = Field(None, description="副本总数量")
    infected_copy_num: int = Field(None, description="感染副本数量")
    uninfected_copy_num: int = Field(None, description="未感染副本数量")
    detecting_copy_num: int = Field(None, description="正在检测副本数量")
    uninspected_copy_num: int = Field(None, description="未检测副本数量")
    prepare_copy_num: int = Field(None, description="准备中副本数量")
    abnormal_copy_num: int = Field(None, description="异常副本数量")


class CopyAntiRansomwareReportSchemas(BaseModel):
    copy_id: str = Field(description="副本ID")
    status: int = Field(description="检测状态")
    model: str = Field(None, description="防勒索检测模型")
    detection_start_time: str = Field(None, description="副本检测开始时间")
    detection_end_time: str = Field(None, description="副本检测完成时间")
    detection_duration: int = Field(None, description="检测持续时间")
    report: str = Field(None, description="防勒索检测报告")


class CopyDetail(BaseModel):
    detail: str = Field(..., description="副本详情")


class CloudBackupCopySchema(CopyInfoSchema):
    deleted: bool = Field(description="副本是否删除")
    gn: int = Field(description="副本GN")


class ModifyCopyAntiRansomwareStatusBody(BaseModel):
    is_security_snap: bool = Field(False, description="防勒索副本是否锁定")


class CopyStatusUpdateByDeviceEsnSchemas(BaseModel):
    device_esn: str = Field(max_length=256, description="副本所在集群设备esn")
    status: CopyStatus = Field(description="副本状态")


class UpdateCopyIndexStatusRequest(BaseModel):
    copy_id_list: List[str] = Field(None, description="副本id列表")
    index_status: str = Field(None, description="待更新索引状态")
    error_code: str = Field(None, description="错误码")
