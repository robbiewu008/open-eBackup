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
package openbackup.data.access.framework.servitization.service.impl;

import openbackup.data.access.framework.servitization.dao.VpcEntityDao;
import openbackup.data.access.framework.servitization.entity.VpcInfoEntity;
import openbackup.data.access.framework.servitization.service.IVpcService;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Set;
import java.util.UUID;

/**
 * VpcServiceImpl
 *
 */
@Service
public class VpcServiceImpl implements IVpcService {
    @Autowired
    private VpcEntityDao vpcEntityDao;

    @Override
    public String saveVpcInfo(String projectId, String vpcId, String markId) {
        LambdaQueryWrapper<VpcInfoEntity> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.eq(VpcInfoEntity::getMarkId, markId);
        VpcInfoEntity vpcInfoEntity = vpcEntityDao.selectOne(queryWrapper);
        boolean isNew = false;
        if (vpcInfoEntity == null) {
            String uuid = UUID.randomUUID().toString();
            vpcInfoEntity = new VpcInfoEntity();
            vpcInfoEntity.setUuid(uuid);
            isNew = true;
        }
        vpcInfoEntity.setMarkId(markId);
        vpcInfoEntity.setProjectId(projectId);
        vpcInfoEntity.setVpcId(vpcId);

        if (isNew) {
            vpcEntityDao.insert(vpcInfoEntity);
        } else {
            vpcEntityDao.updateById(vpcInfoEntity);
        }
        return vpcInfoEntity.getUuid();
    }

    @Override
    public List<VpcInfoEntity> getVpcInfos() {
        return vpcEntityDao.selectList(new LambdaQueryWrapper<>());
    }

    @Override
    public boolean deleteVpcInfo(String markId) {
        LambdaQueryWrapper<VpcInfoEntity> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.eq(VpcInfoEntity::getMarkId, markId);
        int cnt = vpcEntityDao.delete(queryWrapper);
        return cnt > 0;
    }

    @Override
    public VpcInfoEntity getProjectIdByMarkId(String markId) {
        LambdaQueryWrapper<VpcInfoEntity> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.eq(VpcInfoEntity::getMarkId, markId);
        return vpcEntityDao.selectOne(queryWrapper);
    }

    @Override
    public List<VpcInfoEntity> getVpcInfoEntityByProjectId(String userId) {
        LambdaQueryWrapper<VpcInfoEntity> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.eq(VpcInfoEntity::getProjectId, userId);
        return vpcEntityDao.selectList(queryWrapper);
    }

    @Override
    public List<VpcInfoEntity> getVpcByVpcIds(Set<String> vpcIds) {
        LambdaQueryWrapper<VpcInfoEntity> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.in(VpcInfoEntity::getVpcId, vpcIds);
        return vpcEntityDao.selectList(queryWrapper);
    }
}
