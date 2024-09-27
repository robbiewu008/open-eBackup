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

import openbackup.access.framework.resource.persistence.model.ProtectedResourceExtendInfoPo;
import openbackup.access.framework.resource.persistence.model.ResourcesGroupViewPo;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Result;
import org.apache.ibatis.annotations.Results;
import org.apache.ibatis.annotations.Select;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * Protected Resource Extend Info Bo Mapper
 *
 * @author l00272247
 * @since 2021-10-15
 */
@Mapper
@Component
public interface ProtectedResourceExtendInfoMapper extends BaseMapper<ProtectedResourceExtendInfoPo> {
    /**
     * GROUP_EXTEND_SQL
     */
    String GROUP_EXTEND_SQL =
            "select value as value, string_agg(resource_id, ',') as resource_ids " +
                    "from res_extend_info a " +
                    "where a.key = #{key} " +
                    "group by a.value " +
                    "order by a.value ${order} " +
                    "limit #{size} " +
                    "offset #{offset} ";

    /**
     * select by resource id
     *
     * @param id resource id
     * @return ProtectedResourceExtendInfoPo
     */
    @Select("select * from " + ProtectedResourceExtendInfoPo.TABLE_NAME + " where resource_id=#{id}")
    ProtectedResourceExtendInfoPo selectByResourceId(@Param("id") String id);

    /**
     * query page data group by key corresponding value
     *
     * @param size size, the size of data to query
     * @param offset offset, Offset for pagination query
     * @param key key, The content of the key field of the ProtectedResourceExtendInfoPo class
     * @param order order, order by key's value
     * @return List<ResourcesGroupViewPo>
     */
    @Select(GROUP_EXTEND_SQL)
    @Results({
            @Result(column = "value", property = "value"),
            @Result(column = "resource_ids", property = "resourceIds")
    })
    List<ResourcesGroupViewPo> groupByValue(@Param("size") int size,
            @Param("offset") int offset,
            @Param("key") String key,
            @Param("order") String order);

    /**
     * query page data group by key corresponding value
     *
     * @param size size, the size of data to query
     * @param offset offset, Offset for pagination query
     * @param tenantNameFilter tenant name filter
     * @param orderByFs order by filesystem name
     * @return List<ResourcesGroupViewPo>
     */
    @Select(value = "<script>"
        + "select value as value, string_agg(resource_id, ',') as resource_ids "
        + "from res_extend_info a "
        + "where a.key = 'tenantName' "
        + "<if test='tenantNameFilter != null'>"
        + "and a.value like CONCAT('%',#{tenantNameFilter},'%') </if>"
        + "group by a.value "
        + "<if test='orderByFs != null'>"
        + " order by count(value) ${orderByFs} </if>"
        + "limit #{size} offset #{offset}"
        + "</script>")
    @Results({
        @Result(column = "value", property = "value"),
        @Result(column = "resource_ids", property = "resourceIds")
    })
    List<ResourcesGroupViewPo> groupByTenantName(@Param("size") int size,
        @Param("offset") int offset,
        @Param("tenantNameFilter") String tenantNameFilter,
        @Param("orderByFs") String orderByFs);
}
