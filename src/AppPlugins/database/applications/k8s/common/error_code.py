#
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
#

from enum import IntEnum


class ErrorCode(IntEnum):
    # 操作成功
    SUCCESS = 0
    # 错误码编号待定
    # 操作失败
    OPERATION_FAILED = 1677929219
    # 代理内部失败
    ERROR_AGENT_INTERNAL_ERROR = 1577209867
    # 鉴权失败或K8s集群不存在
    AUTH_FAILED = 1577209942
    # K8s返回值为空，查询失败或不存在（尚未评审）
    K8S_API_FAILED = 1577213528
    # 参数缺失
    PARAM_FAILED = 1677929218
    # 获取版本信息失败
    VERSION_FAILED = 1577213512
    # 校验集群失败
    CHECK_CLUSTER_FAILED = 1577209956
    # 连接超时
    ERROR_NETWORK_CONNECT_TIMEOUT = 0x5E025089
    # node selector标签选择的node异常
    NODE_STATUS_IS_ABNORMAL = 1677876737
    # 工作负载下的pod内容器的状态或者命名空间的pod的状态异常
    POD_STATUS_IN_NAMESPACE_OR_DATASET_IS_ABNORMAL = 1577213491
    # pvc的访问模式为ReadWriteOnce，并且pvc绑定的pod不在node selector选中的节点中
    READ_WRITE_ONCE_PVC_BOUND_POD_IS_NOT_ON_SELECTOR_NODE = 1577213521
    # 节点（{0}）上没有可用的CSI Driver。
    NODE_HAS_NO_USE_CSI_DRIVER = 1677876736
    # 错误场景：执行备份任务时，由于未查询到csi插件信息，备份失败。
    NOT_FIND_CSI_PLUGIN = 1577083905
    # 错误场景：执行备份任务时，由于从storage class中未获取到csi-driver，备份失败。
    NOT_FIND_CSI_DRIVER_FROM_STORAGE_CLASS = 1577083904
    # 错误场景：执行备份、恢复任务时，由于创建pod失败，备份、恢复任务失败。
    CREATE_POD_FAILED = 1577083906
    # 错误场景：执行备份、恢复任务时，由于没有可用的逻辑端口，备份、恢复任务失败。
    NO_LOGICAL_PORTS_ARE_AVAILABLE = 1577084160
    # 错误场景：执行备份、恢复任务时，由于添加白名单失败，备份、恢复任务失败。
    ADD_WHITE_LIST_FAILED = 1577083907
    # 错误场景：执行K8s CSI备份时，由于PVC状态异常，操作失败。
    PVC_STATUS_INVALID = 1677876742
