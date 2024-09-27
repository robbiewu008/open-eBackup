/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
