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
package openbackup.access.framework.resource.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceExtendInfoMapper;
import openbackup.access.framework.resource.persistence.model.ProtectedResourceExtendInfoPo;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.model.ProtectedResourceExtendInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.transaction.support.TransactionTemplate;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 */
@Service
@Slf4j
public class ResourceExtendInfoServiceImpl implements ResourceExtendInfoService {
    @Autowired
    private ProtectedResourceExtendInfoMapper protectedResourceExtendInfoMapper;

    @Autowired
    private TransactionTemplate transactionTemplate;

    @Override
    public void deleteByKeys(String resourceId, String... keys) {
        LambdaQueryWrapper<ProtectedResourceExtendInfoPo> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.eq(ProtectedResourceExtendInfoPo::getResourceId, resourceId)
                .in(ProtectedResourceExtendInfoPo::getKey, Arrays.asList(keys));
        protectedResourceExtendInfoMapper.delete(queryWrapper);
    }

    @Override
    @Transactional
    public void saveOrUpdateExtendInfo(String resourceId, Map<String, String> extendInfoMap) {
        for (String key : extendInfoMap.keySet()) {
            LambdaQueryWrapper<ProtectedResourceExtendInfoPo> queryWrapper = new LambdaQueryWrapper<>();
            queryWrapper.eq(ProtectedResourceExtendInfoPo::getResourceId, resourceId)
                .eq(ProtectedResourceExtendInfoPo::getKey, key);
            ProtectedResourceExtendInfoPo infoPo = protectedResourceExtendInfoMapper.selectOne(queryWrapper);
            if (infoPo == null) {
                ProtectedResourceExtendInfoPo resourceExtendInfoPo = new ProtectedResourceExtendInfoPo();
                resourceExtendInfoPo.setUuid(UUID.randomUUID().toString());
                resourceExtendInfoPo.setResourceId(resourceId);
                resourceExtendInfoPo.setKey(key);
                resourceExtendInfoPo.setValue(extendInfoMap.get(key));
                protectedResourceExtendInfoMapper.insert(resourceExtendInfoPo);
            } else {
                QueryWrapper<ProtectedResourceExtendInfoPo> wrapper = new QueryWrapper<>();
                wrapper.eq("uuid", infoPo.getUuid());
                wrapper.last("for update");
                ProtectedResourceExtendInfoPo protectedResourceExtendInfoPo =
                    protectedResourceExtendInfoMapper.selectOne(wrapper);
                protectedResourceExtendInfoPo.setValue(extendInfoMap.get(key));
                protectedResourceExtendInfoMapper.updateById(protectedResourceExtendInfoPo);
            }
        }
    }

    @Override
    public Map<String, String> queryExtendInfo(String resourceId, String... keys) {
        LambdaQueryWrapper<ProtectedResourceExtendInfoPo> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.eq(ProtectedResourceExtendInfoPo::getResourceId, resourceId);
        if (!VerifyUtil.isEmpty(keys) && keys.length > 0) {
            queryWrapper.in(ProtectedResourceExtendInfoPo::getKey, Arrays.asList(keys));
        }
        List<ProtectedResourceExtendInfoPo> protectedResourceExtendInfoPos = protectedResourceExtendInfoMapper
                .selectList(queryWrapper);
        if (VerifyUtil.isEmpty(protectedResourceExtendInfoPos)) {
            return Collections.emptyMap();
        }
        return protectedResourceExtendInfoPos.stream().collect(
                Collectors.toMap(ProtectedResourceExtendInfoPo::getKey, ProtectedResourceExtendInfoPo::getValue));
    }

    @Override
    public List<ProtectedResourceExtendInfo> queryExtendInfo(List<String> resourceIds, String key) {
        if (VerifyUtil.isEmpty(resourceIds)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "query resource extend info fail,reason: input resource id list is empty");
        }
        LambdaQueryWrapper<ProtectedResourceExtendInfoPo> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.in(ProtectedResourceExtendInfoPo::getResourceId, resourceIds)
                .eq(ProtectedResourceExtendInfoPo::getKey, key);
        return protectedResourceExtendInfoMapper.selectList(queryWrapper).stream()
                .map(ProtectedResourceExtendInfoPo::toProtectedResourceExtendInfo).collect(Collectors.toList());
    }

    @Override
    public Map<String, List<ProtectedResourceExtendInfo>> queryExtendInfoByResourceIds(List<String> resourceIds) {
        if (VerifyUtil.isEmpty(resourceIds)) {
            return Collections.emptyMap();
        }
        LambdaQueryWrapper<ProtectedResourceExtendInfoPo> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.in(ProtectedResourceExtendInfoPo::getResourceId, resourceIds);
        List<ProtectedResourceExtendInfo> extendInfoList = protectedResourceExtendInfoMapper.selectList(queryWrapper)
                .stream().map(ProtectedResourceExtendInfoPo::toProtectedResourceExtendInfo)
                .collect(Collectors.toList());
        return extendInfoList.stream()
                .collect(Collectors.groupingBy(ProtectedResourceExtendInfo::getResourceId, Collectors.toList()));
    }
}
