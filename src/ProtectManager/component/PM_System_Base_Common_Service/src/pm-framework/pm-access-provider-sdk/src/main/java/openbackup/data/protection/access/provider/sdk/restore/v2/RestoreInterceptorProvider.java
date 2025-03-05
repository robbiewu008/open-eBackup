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
package openbackup.data.protection.access.provider.sdk.restore.v2;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.enums.AgentMountTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.system.base.common.model.job.JobBo;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 恢复拦截器，该拦截器可针对不同的资源的副本在发送恢复命令到数据保护引擎之前进行拦截，
 * <li>对恢复参数进行扩展，补充和修改</li>
 * <li>对不合法的请求参数进行拦截</li>
 * <li>拦截器由不同的插件提供，并和具体资源的子类型绑定</>
 *
 */
public interface RestoreInterceptorProvider extends DataProtectionProvider<String> {
    /**
     * 拦截恢复请求，对恢复请求进行拦截
     *
     * @param task 恢复参数对象
     * @return 返回恢复任务
     */
    RestoreTask initialize(RestoreTask task);

    /**
     * 获取恢复任务需要锁定的资源
     * <p>
     * 框架会锁定恢复副本，对副本加写锁 </br>
     * 其余需要锁定的资源需要插件自行上报 </br>
     * </p>
     *
     * @param task 恢复任务信息
     * @return 需要锁定的资源列表
     */
    default List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.emptyList();
    }

    /**
     * 恢复任务后置流程(可选)
     *
     * @param task 恢复任务参数
     * @param jobStatus 恢复任务状态
     */
    default void postProcess(RestoreTask task, ProviderJobStatusEnum jobStatus) {
    }

    /**
     * 长时间停止后置流程(可选)
     *
     * @param job 恢复任务
     */
    default void longTimeStopProcess(JobBo job) {
    }

    /**
     * 创建恢复任务前的前置检测
     *
     * @param task 恢复参数对象
     */
    default void restoreTaskCreationPreCheck(RestoreTask task) {
        return;
    }

    /**
     * 创建恢复任务前查询对应环境
     *
     * @param envId envId
     * @return queryEnvironment
     */
    default Optional<ProtectedEnvironment> queryEnvironment(String envId) {
        return Optional.empty();
    }

    /**
     * 特性开关
     *
     * @return RestoreFeature RestoreFeature
     */
    default RestoreFeature getRestoreFeature() {
        return RestoreFeature.defaultValue();
    }

    /**
     * 发送任务到ubc后执行
     *
     * @param task 任务
     */
    default void afterSendTask(RestoreTask task) {
    }

    /**
     * 根据任务返回挂载类型
     *
     * @param task 任务对象
     * @return 挂载类型
     */
    default Optional<AgentMountTypeEnum> getMountType(RestoreTask task) {
        return Optional.empty();
    }

    /**
     * 加密任务扩展信息中敏感信息
     *
     * @param extendInfo 任务扩展信息
     */
    default void encryptExtendInfo(Map<String, String> extendInfo) {
    }

    /**
     * 恢复任务是否支持停止
     *
     * @return 是否支持停止
     */
    default boolean enableStop() {
        return false;
    }
}
