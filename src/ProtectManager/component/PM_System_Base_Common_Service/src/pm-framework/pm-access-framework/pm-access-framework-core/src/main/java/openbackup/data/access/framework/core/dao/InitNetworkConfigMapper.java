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
package openbackup.data.access.framework.core.dao;

import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * 初始化网络配置，操作数据库操作
 *
 */
@Component
public interface InitNetworkConfigMapper {
    /**
     * 查询当前存储认证信息
     *
     * @param initType 初始化类型
     * @return task
     */
    @ExterAttack
    List<InitConfigInfo> queryInitConfig(String initType);

    /**
     * 查询指定设备存储认证信息
     *
     * @param initType 初始化类型
     * @param esn esn
     * @return task
     */
    @ExterAttack
    List<InitConfigInfo> queryInitConfigByEsnAndType(String initType, String esn);

    /**
     * 删除指定类型的配置信息
     *
     * @param initType 初始化类型
     */
    @ExterAttack
    void deleteInitConfig(String initType);

    /**
     * 删除指定设备指定类型的配置信息
     *
     * @param initType 初始化类型
     * @param esn esn
     */
    @ExterAttack
    void deleteInitConfigByEsnAndType(String initType, String esn);

    /**
     * 删除指定设备的配置信息
     *
     * @param esn esn
     */
    @ExterAttack
    void deleteInitConfigByEsn(String esn);

    /**
     * 更新初始化配置信息
     *
     * @param initConfigInfo 存储认证信息
     */
    @ExterAttack
    void updateInitConfig(InitConfigInfo initConfigInfo);

    /**
     * 更新指定设备初始化配置信息
     *
     * @param initConfigInfo 存储认证信息
     */
    @ExterAttack
    void updateInitConfigByEsnAndType(InitConfigInfo initConfigInfo);

    /**
     * 插入初始化配置信息
     *
     * @param initConfigInfo 存储认证信息
     */
    @ExterAttack
    void insertInitConfig(InitConfigInfo initConfigInfo);
}
