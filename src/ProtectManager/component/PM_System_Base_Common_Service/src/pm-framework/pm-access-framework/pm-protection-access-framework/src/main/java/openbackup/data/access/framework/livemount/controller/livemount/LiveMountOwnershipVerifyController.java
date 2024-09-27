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
package openbackup.data.access.framework.livemount.controller.livemount;

import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.livemount.LiveMountOwnershipVerifyRestApi;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

/**
 * Live Mount Ownership Verify Controller
 *
 * @author l00272247
 * @since 2020-11-28
 */
@RestController
@RequestMapping("/v1/internal")
public class LiveMountOwnershipVerifyController implements LiveMountOwnershipVerifyRestApi {
    @Autowired
    private LiveMountEntityDao liveMountEntityDao;

    @Autowired
    private LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    /**
     * verify live mount ownership
     *
     * @param userId user id
     * @param uuidList uuid list
     */
    @Override
    public void verifyLiveMountOwnership(String userId, List<String> uuidList) {
        if (VerifyUtil.isEmpty(uuidList)) {
            return;
        }
        LambdaQueryWrapper<LiveMountEntity> wrapper = new LambdaQueryWrapper<LiveMountEntity>().in(
            LiveMountEntity::getId, uuidList).eq(LiveMountEntity::getUserId, userId);
        int count = liveMountEntityDao.selectCount(wrapper).intValue();
        if (count != uuidList.size()) {
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED);
        }
    }

    /**
     * verify live mount policy ownership
     *
     * @param userId user id
     * @param uuidList uuid list
     */
    @Override
    public void verifyLiveMountPolicyOwnership(String userId, List<String> uuidList) {
        if (VerifyUtil.isEmpty(uuidList)) {
            return;
        }
        LambdaQueryWrapper<LiveMountPolicyEntity> wrapper = new LambdaQueryWrapper<LiveMountPolicyEntity>().in(
            LiveMountPolicyEntity::getPolicyId, uuidList).eq(LiveMountPolicyEntity::getUserId, userId);
        int count = liveMountPolicyEntityDao.selectCount(wrapper).intValue();
        if (count != uuidList.size()) {
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED);
        }
    }
}
