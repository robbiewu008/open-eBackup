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
package openbackup.access.framework.resource.persistence.dao;

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import openbackup.access.framework.resource.persistence.model.VirtualResourceExtendPo;
import openbackup.access.framework.resource.persistence.model.VirtualResourceResponsePo;

import org.apache.ibatis.annotations.Mapper;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * 功能描述
 *
 */
@Mapper
@Component
public interface VirtualResourceMapper extends BaseMapper<VirtualResourceResponsePo> {
    /**
     * queryVirtualResource
     *
     * @param uuid uuid
     * @return VirtualResourceResponsePo 列表
     */
    @ExterAttack
    List<VirtualResourceResponsePo> queryVirtualResource(@Param("uuid") String uuid);


    /**
     * 查询所有虚拟机，匹配过滤规则用，生产实际在4000个以内
     *
     * @param path path
     * @param scopeId scopeId
     * @return VirtualResourceResponsePo 列表
     */
    @ExterAttack
    List<VirtualResourceExtendPo> listAll(@Param("path") String path, @Param("scopeId") String scopeId);
}
