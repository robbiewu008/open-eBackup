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
package openbackup.system.base.common.utils;

import com.baomidou.mybatisplus.core.handlers.MetaObjectHandler;

import lombok.extern.slf4j.Slf4j;

import org.apache.ibatis.reflection.MetaObject;
import org.springframework.stereotype.Component;

import java.sql.Timestamp;

/**
 * MyBatis自动增加创建时间和更新时间货站
 *
 */
@Slf4j
@Component
public class MybatisPlusAutoUpdateTimeUtil implements MetaObjectHandler {
    @Override
    public void insertFill(MetaObject metaObject) {
        log.debug("come to insert fill.");
        this.setFieldValByName("createdTime", new Timestamp(System.currentTimeMillis()), metaObject);
        this.setFieldValByName("updatedTime", new Timestamp(System.currentTimeMillis()), metaObject);
    }

    @Override
    public void updateFill(MetaObject metaObject) {
        log.debug("come to update fill.");
        this.setFieldValByName("updatedTime", new Timestamp(System.currentTimeMillis()), metaObject);
    }
}
