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
package openbackup.system.base.sdk.anti.api;

import openbackup.system.base.sdk.anti.model.AntiRansomwarePolicyRes;
import openbackup.system.base.sdk.anti.model.BatchQueryAntiRansomwarePolicyReq;
import openbackup.system.base.sdk.anti.model.CopyDetectionReq;
import openbackup.system.base.sdk.copy.model.Copy;

import java.util.List;

/**
 * AntiRansomware 本地调用防勒索API接口定义
 *
 */
public interface AntiRansomwareApi {
    /**
     * 查询防勒索策略
     *
     * @param resourceId 资源id
     * @return AntiRansomwarePolicyRes 防勒索策略
     */
    AntiRansomwarePolicyRes getPolicyByResourceId(String resourceId);

    /**
     * 副本防勒索检测
     *
     * @param copyDetectionReq 检测信息
     * @return 副本信息
     */
    Copy createInternalCopyDetection(CopyDetectionReq copyDetectionReq);

    /**
     * 根据资源id列表查询防勒索策略
     *
     * @param req 批量查询防勒索策略请求
     * @return 防勒索策略列表
     */
    List<AntiRansomwarePolicyRes> batchQueryPolicies(BatchQueryAntiRansomwarePolicyReq req);

    /**
     * 是否存在
     *
     * @param policyId 策略id
     * @return boolean
     */
    boolean existPolicy(String policyId);

    /**
     * 策略id列表
     *
     * @return 策略id列表
     */
    List<Integer> getAntiPolicyIdList();

    /**
     *  获取 防勒索&WORM策略 总数量
     *
     * @return 防勒索&WORM策略 的总数量
     */
    Integer getAllCount();

    /**
     * 根据资源id查询worm开关是否开启
     *
     * @param resourceId 资源id
     * @return 该资源id对应的worm开关是否开启
     */
    boolean isExistWormPolicyByResourceId(String resourceId);
}
