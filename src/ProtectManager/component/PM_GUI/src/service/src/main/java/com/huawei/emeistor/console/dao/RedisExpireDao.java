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
package com.huawei.emeistor.console.dao;

import com.huawei.emeistor.console.bean.RedisExpireEntity;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Param;

import java.util.List;

/**
 * redis过期数据库dao
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/5/24
 */
public interface RedisExpireDao extends BaseMapper<RedisExpireEntity> {
    /**
     * 通过类型查询redis键过期信息
     *
     * @param type redis键过期类型
     * @return redis键过期信息
     */
    List<RedisExpireEntity> getRedisExpireInfoByType(@Param("type") String type);

    /**
     * 通过key删除redis键过期信息
     *
     * @param key redis键
     */
    void deleteRedisExpireInfoByKey(@Param("key") String key);

    /**
     * 插入redis键过期信息
     *
     * @param redisExpireEntity redis键过期信息实体类
     */
    void insertRedisExpireInfo(RedisExpireEntity redisExpireEntity);
}
