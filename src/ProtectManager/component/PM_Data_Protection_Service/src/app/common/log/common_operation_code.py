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
class OperationAlarmCode:
    # 创建限速策略
    CREATE_QOS = "0x206403350002"

    # 删除限速策略
    DELETE_QOS = "0x206403350003"

    # 更新限速策略
    UPDATE_QOS = "0x206403350004"

    # 删除副本
    DELETE_COPY = "0x2064033A0001"

    # 删除快照-安全一体机
    DELETE_COPY_CYBER = "0x2064033A000A"

    # 更新副本过期时间
    UPDATE_COPY_RETENTION = "0x2064033A0002"

    # 更新快照保留策略-安全一体机
    UPDATE_COPY_RETENTION_CYBER = "0x2064033A000B"

    # 创建副本引索
    CREATE_COPY_INDEX = "0x2064033A0003"

    # 修改VMware注册信息
    MODIFY_VMWARE_REGISTRATION_INFO = "0x206403320015"

    # 移除VMware
    REMOVE_VMWARE = "0x206403320016"

    # 注册VMware
    REGISTER_VMWARE = "0x206403320017"

    # 重新扫描VMware信息
    RESCAN_VMWARE_INFO = "0x206403320018"

    # 副本防勒索误报处理
    RANSOMWARE_DETECTION_FALSE_ALARM_HANDLED = "0x2064033A0006"

    # 处理智能侦测误报-安全一体机
    RANSOMWARE_DETECTION_FALSE_ALARM_HANDLED_CYBER = "0x2064033A000C"

    # 清除资源的副本索引
    DELETE_RESOURCE_COPY_INDEX = "0x2064033A0007"

    # 更新WORM设置
    UPDATE_WORM_COPY_SETTING = "0x2064032B0027"

