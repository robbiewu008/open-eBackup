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
package openbackup.data.protection.access.provider.sdk.copy;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.enums.AgentMountTypeEnum;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.system.base.sdk.copy.model.Copy;

import java.util.Optional;

/**
 * 副本校验拦截器
 *
 */
public interface CopyVerifyInterceptor extends DataProtectionProvider<String> {
    /**
     * 副本校验拦截方法
     *
     * @param task 副本校验任务
     * @return CopyVerifyTask
     */
    CopyVerifyTask interceptor(CopyVerifyTask task);

    /**
     * 副本校验复制副本不允许副本校验不支持的副本类型列表
     *
     * @param copy 副本对象
     */
    void checkIsSupportVerify(Copy copy);

    /**
     * 副本校验所使用的挂载类型
     *
     * @param task 任务对象
     * @return agent挂载类型
     */
    default Optional<AgentMountTypeEnum> getMountType(CopyVerifyTask task) {
        return Optional.empty();
    }
}
