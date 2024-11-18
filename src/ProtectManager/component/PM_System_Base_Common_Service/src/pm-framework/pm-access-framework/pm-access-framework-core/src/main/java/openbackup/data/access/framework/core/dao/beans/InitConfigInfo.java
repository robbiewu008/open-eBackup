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
package openbackup.data.access.framework.core.dao.beans;

import com.baomidou.mybatisplus.annotation.FieldStrategy;
import com.baomidou.mybatisplus.annotation.TableField;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 初始化配置，与数据库对应
 *
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class InitConfigInfo {
    /**
     * 初始化类型
     */
    private String initType;

    /**
     * 初始化值
     */
    private String initValue;

    /**
     * 初始化配置创建时间
     */
    @TableField(updateStrategy = FieldStrategy.IGNORED)
    private Long createTime;

    /**
     * esn
     */
    private String esn;

    public InitConfigInfo(String initType, String initValue) {
        this.initType = initType;
        this.initValue = initValue;
    }

    /**
     * 三参构造函数
     *
     * @param initType 类型
     * @param initValue 值
     * @param createTime 创建时间
     */
    public InitConfigInfo(String initType, String initValue, Long createTime) {
        this(initType, initValue);
        this.createTime = createTime;
    }
}