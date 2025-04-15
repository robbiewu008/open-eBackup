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

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.springframework.stereotype.Component;

/**
 * Protected Object Mapper
 *
 */
@Mapper
@Component
public interface ProtectedObjectMapper extends BaseMapper<ProtectedObjectPo> {
    /**
     * 根据SLA的ID查询关联资源数量【内部接口】
     *
     * @param slaName SLA的Name
     * @param userId 用户Id
     * @param subType 资源子类型
     * @return 数量
     */
    int countBySubTypeAndSlaName(@Param("slaName") String slaName, @Param("userId") String userId,
            @Param("subType") String subType);

    /**
     * 根据SLA的ID查询关联资源数量【内部接口】
     *
     * @param slaId slaId
     * @return 数量
     */
    int countBySlaId(@Param("slaId") String slaId);

    /**
     * 手动保存保护对象
     *
     * @param protectedObjectPo protectedObjectPo
     * @return 数量
     */
    @ExterAttack
    int insertProtectedObject(@Param("po") ProtectedObjectPo protectedObjectPo);

    /**
     * 更新json 类型的 extParameters
     *
     * @param protectedObjectPo  protectedObjectPo
     */
    @ExterAttack
    void updateExtParameters(@Param("po") ProtectedObjectPo protectedObjectPo);
}
