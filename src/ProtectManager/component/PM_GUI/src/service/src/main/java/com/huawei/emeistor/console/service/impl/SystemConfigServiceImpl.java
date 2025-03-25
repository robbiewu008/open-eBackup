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
package com.huawei.emeistor.console.service.impl;

import com.huawei.emeistor.console.dao.SystemConfigMapper;
import com.huawei.emeistor.console.dao.model.SystemConfigEntity;
import com.huawei.emeistor.console.kmchelp.KmcHelper;
import com.huawei.emeistor.console.service.SystemConfigService;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.Map;
import java.util.UUID;

/**
 * SystemConfigService实现类
 *
 */
@Service
public class SystemConfigServiceImpl implements SystemConfigService {
    @Autowired
    private SystemConfigMapper systemConfigMapper;

    /**
     * 保存或更新配置信息，该方法整体有事务
     *
     * @param configMap 配置信息
     */
    @Transactional(rollbackFor = Exception.class)
    @Override
    public void saveOrUpdateConfig(Map<String, String> configMap) {
        for (Map.Entry<String, String> entry : configMap.entrySet()) {
            saveOrUpdateRsaContent(entry.getKey(), entry.getValue());
        }
    }

    private void saveOrUpdateRsaContent(String key, String content) {
        SystemConfigEntity systemConfigEntity = systemConfigMapper.selectOne(new QueryWrapper<SystemConfigEntity>()
                .lambda()
                .eq(SystemConfigEntity::getConfigKey, key));
        if (systemConfigEntity == null) {
            systemConfigEntity = new SystemConfigEntity();
            systemConfigEntity.setUuid(UUID.randomUUID().toString());
            systemConfigEntity.setConfigKey(key);
            systemConfigEntity.setConfigValue(content);
            systemConfigMapper.insert(systemConfigEntity);
        } else {
            systemConfigEntity.setConfigValue(content);
            systemConfigMapper.updateById(systemConfigEntity);
        }
    }

    /**
     * 查询配置信息
     *
     * @param key key
     * @param isNeedDecrypt 是否需要解密
     * @return value
     */
    @Override
    public String queryConfig(String key, boolean isNeedDecrypt) {
        SystemConfigEntity systemConfigEntity = systemConfigMapper.selectOne(new QueryWrapper<SystemConfigEntity>()
                .lambda()
                .eq(SystemConfigEntity::getConfigKey, key));
        if (systemConfigEntity == null) {
            return "";
        }
        if (isNeedDecrypt) {
            return KmcHelper.getInstance().decrypt(systemConfigEntity.getConfigValue());
        }
        return systemConfigEntity.getConfigValue();
    }
}
