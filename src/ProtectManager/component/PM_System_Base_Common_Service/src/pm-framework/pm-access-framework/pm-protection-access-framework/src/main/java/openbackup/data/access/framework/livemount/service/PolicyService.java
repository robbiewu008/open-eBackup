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
package openbackup.data.access.framework.livemount.service;

import openbackup.data.access.framework.livemount.controller.policy.request.CreatePolicyRequest;
import openbackup.data.access.framework.livemount.controller.policy.request.UpdatePolicyRequest;
import openbackup.data.access.framework.livemount.controller.policy.response.LiveMountPolicyVo;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.sdk.copy.model.BasePage;

import java.util.List;

/**
 * 更新策略接口
 *
 * @author h30003246
 * @since 2020-09-18
 */
public interface PolicyService {
    /**
     * create a live mount policy
     *
     * @param createRequest create request
     * @param user user
     */
    void createPolicy(CreatePolicyRequest createRequest, TokenBo.UserBo user);

    /**
     * query live mount policy
     *
     * @param page page
     * @param size size
     * @param conditions conditions
     * @param orders order
     * @return live mount policy
     */
    BasePage<LiveMountPolicyEntity> getPolices(int page, int size, String conditions, List<String> orders);

    /**
     * query a live mount policy by id
     *
     * @param id id
     * @return updating policy
     */
    LiveMountPolicyVo getPolicy(String id);

    /**
     * update a live mount policy
     *
     * @param policyId policy id
     * @param updateRequest update request
     */
    void updatePolicy(String policyId, UpdatePolicyRequest updateRequest);

    /**
     * delete a  live mount policy
     *
     * @param policyId policy id
     */
    void deletePolicy(String policyId);

    /**
     * get a live mount policy by id
     *
     * @param policyId policy id
     * @return policy entity
     */
    LiveMountPolicyEntity selectPolicyById(String policyId);

    /**
     * revoke live mount policy user id
     *
     * @param userId user id
     */
    void revokeLiveMountPolicyUserId(String userId);
}
