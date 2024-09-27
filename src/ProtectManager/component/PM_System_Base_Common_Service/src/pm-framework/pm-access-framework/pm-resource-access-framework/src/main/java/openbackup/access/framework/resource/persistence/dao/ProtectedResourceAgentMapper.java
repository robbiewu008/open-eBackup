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

import openbackup.access.framework.resource.persistence.model.ProtectedEnvironmentPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Mapper;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;

/**
 * agent主机mybatis Mapper
 *
 * @author z00613137
 * @since 2023-08-08
 */
@Mapper
@Component
public interface ProtectedResourceAgentMapper extends BaseMapper<ProtectedResourcePo> {
    /**
     * 根据条件查询主机资源列表
     *
     * @param map 查询条件
     * @return 资源列表
     */
    List<ProtectedEnvironmentPo> queryAgentResourceList(Map<String, Object> map);

    /**
     * 根据条件查询主机资源的总数
     *
     * @param map 查询条件
     * @return 资源列表
     */
    int queryAgentResourceCount(Map<String, Object> map);

    /**
     * 查询共享Agent
     *
     * @return 资源列表
     */
    List<String> querySharedAgentIds();
}
