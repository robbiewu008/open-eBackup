/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#pragma once

#include <string>
#include "KubeMacros.h"

KUBERNETES_PLUGIN_NAMESPACE_BEGIN
/**
 * 级别：INFO
 * 描述：开始对POD（{0}）进行备份。
 * 阶段：前置任务
 */
const std::string MARK_POD_BACKUP_SUCCESS_LABEL = "virtual_plugin_k8s_mark_pod_backup_success_label";
/**
 * 级别：INFO
 * 描述：在POD（{0}）的容器（{1}）中备份/恢复前执行脚本（{2}）执行成功（{3}）。
 * 阶段：前置任务
 */
const std::string PRE_SCRIPT_EXEC_SUCCESS_LABEL = "virtual_plugin_k8s_pre_script_exec_success_label";
/**
 * 级别：INFO
 * 描述：找到需要备份的POD({0})，备份其挂载卷（{1}）。
 * 阶段：前置任务
 */
const std::string SELECTED_BACKUP_VOLUME_LABEL = "virtual_plugin_k8s_selected_backup_volume_label";
/**
 * 级别：INFO
 * 描述：在时间点（{0}），对生产存储快照（{1}）一致性激活成功。
 * 阶段：前置任务
 */
const std::string ACTIVATE_SNAPS_SUCCESS_LABEL = "virtual_plugin_k8s_activite_snaps_success_label";
/**
 * 级别：INFO
 * 描述：删除生产存储快照（{0}）成功。
 * 阶段：后置任务
 */
const std::string DELETE_SNAP_SUCCESS_LABEL = "virtual_plugin_k8s_delete_snap_success_label";
/**
 * 级别：INFO
 * 描述：在POD（{0}）的容器（{1}）中备份/恢复成功执行脚本（{2}）执行成功。
 * 阶段：后置任务
 */
const std::string POST_SCRIPT_EXEC_SUCCESS_LABEL = "virtual_plugin_k8s_post_script_exec_success_label";
/**
 * 级别：INFO
 * 描述：在POD（{0}）的容器（{1}）中备份/恢复失败执行脚本（{2}）执行成功。
 * 阶段：后置任务
 */
const std::string FAILED_SCRIPT_EXEC_SUCCESS_LABEL = "virtual_plugin_k8s_failed_script_exec_success_label";
/**
 * 级别：ERROR
 * 描述：Statefulset（{0}）状态异常。
 * 阶段：前置任务
 */
const std::string STATEFULSET_ABNORMAL_LABEL = "virtual_plugin_k8s_statefulset_abnormal_label";
/**
 * 级别：ERROR
 * 描述：没有找到需要备份的POD。
 * 阶段：前置任务
 */
const std::string BACKUP_PODS_FILTER_EMPTY_LABEL = "virtual_plugin_k8s_backup_pods_filter_empty_label";
/**
 * 级别：ERROR
 * 描述：PVC({0})的状态异常。
 * 阶段：前置任务
 */
const std::string PVC_STATUS_PENDING_LABEL = "virtual_plugin_k8s_pvc_status_pending_label";
/**
 * 级别：ERROR
 * 描述：在POD（{0}）的容器（{1}）中的备份/恢复前执行脚本（{2}）执行失败。
 * 阶段：前置任务
 */
const std::string PRE_SCRIPT_EXEC_FAIL_LABEL = "virtual_plugin_k8s_pre_script_exec_fail_label";
/**
 * 级别：WARN
 * 描述：删除生产存储快照（{0}）失败。
 * 阶段：后置任务
 */
const std::string DELETE_SNAP_FAIL_LABEL = "virtual_plugin_k8s_delete_snap_fail_label";
/**
 * 级别：WARN
 * 描述：在POD（{0}）的容器（{1}）中的备份/恢复成功执行脚本（{2}）执行失败。
 * 阶段：后置任务
 */
const std::string POST_SCRIPT_EXEC_FAIL_LABEL = "virtual_plugin_k8s_post_script_exec_fail_label";
/**
 * 级别：WARN
 * 描述：在POD（{0}）的容器（{1}）中的备份/恢复失败执行脚本（{2}）执行失败。
 * 阶段：后置任务
 */
const std::string FAILED_SCRIPT_EXEC_FAIL_LABEL = "virtual_plugin_k8s_failed_script_exec_fail_label";
/**
 * 级别：ERROR
 * 描述：获取卷（{0}）所对应的PV信息失败。
 * 阶段：前置任务
 */
const std::string FAILED_GET_PV_CORRESPONDING_TO_VOLUME_LABEL = "virtual_plugin_k8s_get_pv_failed_label";

/**
 * 级别：ERROR
 * 描述：在POD（{0}）的容器（{1}）中无备份/恢复前执行脚本（{2}）权限。
 * 阶段：前置任务
 */
const std::string PRE_SCRIPT_NO_PERMISSION_EXEC_LABEL = "virtual_plugin_k8s_no_permission_exec_pre_script_label";
/**
 * 级别：WARN
 * 描述：在POD（{0}）的容器（{1}）中无备份/恢复成功执行脚本（{2}）权限。
 * 阶段：后置任务
 */
const std::string POST_SCRIPT_NO_PERMISSION_EXEC_LABEL = "virtual_plugin_k8s_no_permission_exec_post_script_label";
/**
 * 级别：WARN
 * 描述：在POD（{0}）的容器（{1}）中无备份/恢复失败执行脚本（{2}）权限。
 * 阶段：后置任务
 */
const std::string BACKUP_FAILED_NO_PERMISSION_EXEC_FAILED_SCRIPT_LABEL =
    "virtual_plugin_k8s_no_permission_exec_backup_failed_script_label";
/**
 * 级别：ERROR
 * 描述：在POD（{0}）的容器（{1}）中的备份/恢复前执行脚本（{2}）格式错误。
 * 阶段：前置任务
 */
const std::string PRE_SCRIPT_FORMAT_ERROR_LABEL = "virtual_plugin_k8s_pre_script_format_error_label";
/**
 * 级别：WARN
 * 描述：在POD（{0}）的容器（{1}）中的备份/恢复成功执行脚本（{2}）格式错误。
 * 阶段：后置任务
 */
const std::string POST_SCRIPT_FORMAT_ERROR_LABEL = "virtual_plugin_k8s_post_script_format_error_label";
/**
 * 级别：WARN
 * 描述：在POD（{0}）的容器（{1}）中的备份/恢复失败执行脚本（{2}）格式错误。
 * 阶段：后置任务
 */
const std::string BACKUP_FAILED_EXEC_SCRIPT_FORMAT_ERROR_LABEL =
    "virtual_plugin_k8s_backup_failed_exec_script_format_error_label";
/**
 * 级别：ERROR
 * 描述：在POD（{0}）的容器（{1}）中未找到备份/恢复前执行脚本（{2}）。
 * 阶段：前置任务
 */
const std::string PRE_SCRIPT_NOT_FOUND_LABEL = "virtual_plugin_k8s_pre_script_not_found_label";

/**
 * 级别：WARN
 * 描述：在POD（{0}）的容器（{1}）中未找到备份/恢复成功执行脚本（{2}）。
 * 阶段：后置任务
 */
const std::string POST_SCRIPT_NOT_FOUND_LABEL = "virtual_plugin_k8s_post_script_not_found_label";
/**
 * 级别：WARN
 * 描述：在POD（{0}）的容器（{1}）中未找到备份/恢复失败执行脚本（{2}）。
 * 阶段：后置任务
 */
const std::string BACKUP_FAILED_EXEC_SCRIPT_NOT_FOUND_LABEL =
        "virtual_plugin_k8s_backup_failed_exec_script_not_found_label";

/**
 * 级别：ERROR
 * 描述：缩容POD至0失败。
 * 阶段：后置任务
 */
const std::string SCALE_THE_PODS_TO_ZERO_FAILED_LABEL = "virtual_plugin_k8s_scale_the_pods_to_zero_failed_label";

/**
 * 级别：ERROR
 * 描述：代理主机缺少必需工具：{}。
 * 阶段：前置任务
 */
const std::string LACK_NEED_TOOL_LABEL = "virtual_plugin_k8s_lack_tools_failed_label";

/**
 * 级别：ERROR
 * 描述：存储设备({})存储池({})剩余空间不满足阈值({})。
 * 阶段：前置任务
 */
const std::string SPACE_NOT_ABLE_LABEL = "virtual_plugin_k8s_storage_space_failed_label";
KUBERNETES_PLUGIN_NAMESPACE_END