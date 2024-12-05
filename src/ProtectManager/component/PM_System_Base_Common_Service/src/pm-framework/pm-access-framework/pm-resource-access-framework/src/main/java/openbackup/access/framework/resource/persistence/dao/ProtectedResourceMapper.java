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

import com.baomidou.mybatisplus.core.conditions.Wrapper;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.baomidou.mybatisplus.core.metadata.IPage;
import com.baomidou.mybatisplus.core.toolkit.Constants;

import openbackup.access.framework.resource.dto.ResourceDependencyRelation;
import openbackup.access.framework.resource.persistence.model.ProtectedEnvironmentPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.security.callee.CalleeMethod;
import openbackup.system.base.security.callee.CalleeMethods;

import org.apache.ibatis.annotations.Many;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.One;
import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Result;
import org.apache.ibatis.annotations.Results;
import org.apache.ibatis.annotations.Select;
import org.apache.ibatis.annotations.Update;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;

/**
 * 受保护资源mybatis Mapper
 *
 */
@Mapper
@Component
@CalleeMethods(
        name = "resource_dao",
        value = {@CalleeMethod(name = "selectBatchIds"), @CalleeMethod(name = "selectById")})
public interface ProtectedResourceMapper extends BaseMapper<ProtectedResourcePo> {
    /**
     * RESOURCE_SQL
     */
    String RESOURCE_SQL =
            "select r.*, e.endpoint, e.port, e.user_name, e.password, e.link_status, e.location, e.os_type, e.os_name,"
                    + " e.scan_interval, e.is_cluster, e.cert_name, e.time_zone from resources as r"
                    + " left join environments as e on r.discriminator = 'environments' and r.uuid = e.uuid"
                    + " left join protected_object po on po.uuid = r.uuid"
                    + " left join resources as rr on rr.discriminator = 'environments' and r.root_uuid = rr.uuid"
                    + " left join environments as re on r.root_uuid = re.uuid"
                    + " ${extendCascades} ${ew.customSqlSegment}";

    /**
     * ID_LIST
     */
    String ID_LIST = "<foreach collection='ids' item='id' open='(' separator=',' close=')'>#{id}</foreach>";

    /**
     * EX_LIST
     */
    String EX_LIST = "<foreach collection='exs' item='ex' open='(' separator=',' close=')'>#{ex}</foreach>";

    /**
     * WITH RECURSIVE tree(uuid, parent_uuid) as ( <br/>
     * select r1.uuid, r1.parent_uuid from resources r1 where r1.uuid in(?)<br/>
     * union all<br/>
     * select r2.uuid, r2.parent_uuid from resources r2, tree r0<br/>
     * where (r2.root_uuid = r0.uuid or r2.parent_uuid = r0.uuid) and r2.uuid not in(?)
     * <br/>
     * ) select uuid from tree<br/>
     */
    String RELATED_RESOURCE_SQL =
            "<script>WITH RECURSIVE tree(uuid, parent_uuid) as ("
                    + "select r1.uuid, r1.parent_uuid from resources r1 where <choose>"
                    + "<when test='ids != null and ids.size > 0'>r1.uuid in"
                    + ID_LIST
                    + "</when>"
                    + "<otherwise><![CDATA[ 1<>1 ]]></otherwise></choose>"
                    + " union all select r2.uuid, r2.parent_uuid from resources r2, tree r0 "
                    + "where ((r2.root_uuid = r0.uuid or r2.parent_uuid = r0.uuid) and r2.uuid<![CDATA[ <> ]]>r0.uuid)"
                    + "<if test='exs != null and exs.size > 0'> and r2.uuid not in"
                    + EX_LIST
                    + "</if>"
                    + ") select uuid from tree</script>";

    /**
     * RESOURCE_SQL
     */
    String DEPENDENCY_RESOURCE_SQL =
        "<script>select r.*, e.endpoint, e.port, e.user_name, e.password, e.link_status, e.location, e.os_type, "
            + "e.os_name,e.scan_interval, e.is_cluster, e.cert_name, e.time_zone from resources as r"
            + " left join environments as e on r.discriminator = 'environments' and r.uuid = e.uuid"
            + " left join protected_object po on po.uuid = r.uuid"
            + " left join resources as rr on rr.discriminator = 'environments' and r.root_uuid = rr.uuid"
            + " left join environments as re on r.root_uuid = re.uuid where r.uuid in "
            + "(select distinct resource_id from RES_EXTEND_INFO <where>"
            + "<if test='key != null'> and key like '$citations_'||#{key}||'%'</if>"
            + "<if test='ids != null and ids.size > 0'> and value in" + ID_LIST + "</if></where>)</script>";

    /**
     * EXIST_RESOURCE_SQL
     */
    String EXIST_RESOURCE_SQL = "<script>select uuid from resources where uuid in" + ID_LIST + "</script>";

    /**
     * RESOURCE_UUID_BY_SOURCE_TYPE_SQL
     */
    String RESOURCE_UUID_BY_SOURCE_TYPE_SQL = "<script>select uuid from resources where uuid in" + ID_LIST
        + " and source_type = #{sourceType}"
        + "</script>";

    /**
     * RESOURCE_UUID_BY_ROOT_UUID_CYBER_ENGINE_SQL
     */
    String RESOURCE_UUID_BY_ROOT_UUID_CYBER_ENGINE_SQL = "<script>select uuid from resources where "
            + "root_uuid = #{rootUuid} and "
            + "(SUB_TYPE = 'Tenant' or SUB_TYPE = 'CloudBackupFileSystem' or SUB_TYPE = 'FileSystem')"
            + "</script>";

    /**
     * RESOURCE_UUID_SQL
     */
    String RESOURCE_UUID_SQL = "select r.uuid from resources r ${ew.customSqlSegment}";

    /**
     * WITH RECURSIVE tree(uuid, parent_uuid) as (<br/>
     * select r1.uuid, r1.parent_uuid from resources r1 where r1.uuid = #{id}<br/>
     * union all<br/>
     * select r2.uuid, r2.parent_uuid from resources r2, tree r0<br/>
     * where r0.parent_uuid is not null and r2.uuid = r0.parent_uuid<br/>
     * ) select uuid from tree where parent_uuid is null<br/>
     */
    String ROOT_UUID_SQL =
            "WITH RECURSIVE tree(uuid, parent_uuid) as ("
                    + "select r1.uuid, r1.parent_uuid from resources r1 where r1.uuid = #{id} union all "
                    + "select r2.uuid, r2.parent_uuid from resources r2, tree r0 "
                    + "where r0.parent_uuid is not null and r2.uuid = r0.parent_uuid and r2.uuid <> r0.uuid"
                    + ") select uuid from tree where parent_uuid is null";

    /**
     * dependency relation query sql
     */
    String DEPENDENCY_RELATION_SQL = "<script>"
        + "WITH RECURSIVE tree(uuid, type, parent_uuid) AS "
        + "                   (SELECT r1.uuid,'children' AS type, r1.parent_uuid "
        + "                    FROM resources r1" + "                    WHERE "
        + "<choose>"
        + "<when test='ids != null and ids.size > 0'>r1.uuid in"
        + ID_LIST
        + "</when>"
        + "<otherwise><![CDATA[ 1<>1 ]]></otherwise></choose>"
        + "                    UNION all SELECT r2.uuid, "
        + "                                   'children' AS type, r2.parent_uuid "
        + "                    FROM resources r2, tree r0 " + "                    WHERE ((r2.root_uuid = r0.uuid "
        + "                        OR r2.parent_uuid = r0.uuid) "
        + "                        AND r2.uuid !=r0.uuid) ) " + "SELECT uuid, TYPE, parent_uuid "
        + "FROM tree " + " UNION " + "SELECT r3.resource_id uuid, " + "       r3.key as type, "
        + "       r3.value parent_uuid " + "FROM RES_EXTEND_INFO r3, tree r4 " + "WHERE r3.value = r4.uuid "
        + "  AND r3.key LIKE '$citations_%'"
        + "</script>";

    /**
     * desesitization query sql
     */
    String DESESITIZATION_RESOURCE_SQL =
        "select r.*, e.endpoint, e.port, e.user_name, e.password, e.link_status, e.location, e.os_type, e.os_name,"
            + " e.scan_interval, e.is_cluster, e.cert_name, e.time_zone from resources as r"
            + " left join environments as e on r.discriminator = 'environments' and r.uuid = e.uuid"
            + " left join resource_desesitization rd on rd.uuid = r.uuid"
            + " left join resources as rr on rr.discriminator = 'environments' and r.root_uuid = rr.uuid"
            + " left join environments as re on r.root_uuid = re.uuid"
            + " ${extendCascades} ${ew.customSqlSegment}";

    /**
     * paginate method
     *
     * @param page page
     * @param queryWrapper query wrapper
     * @param extendCascades extendCascades
     * @return page data
     */
    @Select(DESESITIZATION_RESOURCE_SQL)
    @Results({
        @Result(id = true, column = "uuid", property = "uuid"),
        @Result(column = "name", property = "name"),
        @Result(column = "type", property = "type"),
        @Result(column = "sub_type", property = "subType"),
        @Result(column = "path", property = "path"),
        @Result(column = "created_time", property = "createdTime"),
        @Result(column = "parent_name", property = "parentName"),
        @Result(column = "parent_uuid", property = "parentUuid"),
        @Result(column = "root_uuid", property = "rootUuid"),
        @Result(column = "protection_status", property = "protectionStatus"),
        @Result(column = "source_Type", property = "sourceType"),
        @Result(column = "user_id", property = "userId"),
        @Result(column = "authorized_user", property = "authorizedUser"),
        @Result(column = "user_name", property = "username"),
        @Result(
            column = "uuid",
            property = "extendInfoList",
            many =
            @Many(
                select =
                    "openbackup.access.framework.resource.persistence.dao."
                        + "ProtectedResourceExtendInfoMapper.selectByResourceId")),
        @Result(
            column = "uuid",
            property = "desesitizationPo",
            one =
            @One(
                select =
                    "openbackup.access.framework.resource.persistence.dao."
                        + "ResourceDesesitizationMapper.selectById"))
    })
    IPage<ProtectedEnvironmentPo> desesitizationPaginate(
        IPage<ProtectedResourcePo> page,
        @Param(Constants.WRAPPER) QueryWrapper<ProtectedResourcePo> queryWrapper,
        @Param("extendCascades") String extendCascades);

    /**
     * paginate method
     *
     * @param page page
     * @param queryWrapper query wrapper
     * @param extendCascades extendCascades
     * @return page data
     */
    @Select(RESOURCE_SQL)
    @Results({
        @Result(id = true, column = "uuid", property = "uuid"),
        @Result(column = "name", property = "name"),
        @Result(column = "type", property = "type"),
        @Result(column = "sub_type", property = "subType"),
        @Result(column = "path", property = "path"),
        @Result(column = "created_time", property = "createdTime"),
        @Result(column = "parent_name", property = "parentName"),
        @Result(column = "parent_uuid", property = "parentUuid"),
        @Result(column = "root_uuid", property = "rootUuid"),
        @Result(column = "protection_status", property = "protectionStatus"),
        @Result(column = "source_Type", property = "sourceType"),
        @Result(column = "user_id", property = "userId"),
        @Result(column = "authorized_user", property = "authorizedUser"),
        @Result(column = "user_name", property = "username"),
        @Result(
                column = "uuid",
                property = "extendInfoList",
                many =
                        @Many(
                                select =
                                        "openbackup.access.framework.resource.persistence.dao."
                                                + "ProtectedResourceExtendInfoMapper.selectByResourceId")),
        @Result(
                column = "uuid",
                property = "protectedObjectPo",
                one =
                        @One(
                                select =
                                        "openbackup.data.access.framework.core.dao."
                                                + "ProtectedObjectMapper.selectById"))
    })
    IPage<ProtectedEnvironmentPo> paginate(
            IPage<ProtectedResourcePo> page,
            @Param(Constants.WRAPPER) QueryWrapper<ProtectedResourcePo> queryWrapper,
            @Param("extendCascades") String extendCascades);

    /**
     * dependency resources query
     *
     * @param key dependency key
     * @param uuids resource uuid
     * @return dependency resources
     */
    @Select(DEPENDENCY_RESOURCE_SQL)
    @Results({
        @Result(id = true, column = "uuid", property = "uuid"),
        @Result(column = "name", property = "name"),
        @Result(column = "type", property = "type"),
        @Result(column = "sub_type", property = "subType"),
        @Result(column = "path", property = "path"),
        @Result(column = "created_time", property = "createdTime"),
        @Result(column = "parent_name", property = "parentName"),
        @Result(column = "parent_uuid", property = "parentUuid"),
        @Result(column = "root_uuid", property = "rootUuid"),
        @Result(column = "protection_status", property = "protectionStatus"),
        @Result(column = "source_Type", property = "sourceType"),
        @Result(column = "user_id", property = "userId"),
        @Result(column = "authorized_user", property = "authorizedUser"),
        @Result(column = "user_name", property = "username"),
        @Result(
            column = "uuid",
            property = "extendInfoList",
            many =
            @Many(
                select =
                    "openbackup.access.framework.resource.persistence.dao."
                        + "ProtectedResourceExtendInfoMapper.selectByResourceId")),
        @Result(
            column = "uuid",
            property = "protectedObjectPo",
            one =
            @One(
                select =
                    "openbackup.data.access.framework.core.dao."
                        + "ProtectedObjectMapper.selectById"))
    })
    List<ProtectedEnvironmentPo> queryDependencyResources(@Param("key") String key, @Param("ids") List<String> uuids);

    /**
     * 查询用户名下资源
     *
     * @param userId 用户id
     * @return 资源列表
     */
    List<ProtectedEnvironmentPo> queryResourcesByUserId(@Param("userId") String userId);

    /**
     * 查看资源详情
     *
     * @param uuid 资源id
     * @return 资源
     */
    ProtectedEnvironmentPo queryResourceById(@Param("uuid") String uuid);

    /**
     * query sub resource uuid list
     *
     * @param uuidList resource uuid list
     * @return sub resource uuid list
     */
    default List<String> queryRelatedResourceUuids(@Param("ids") List<String> uuidList) {
        return queryRelatedResourceUuids(uuidList, Collections.emptyList());
    }

    /**
     * query related resource uuid
     *
     * @param parentUuids parent resource uuids
     * @param excludeResourceUuids excluded resource uuids
     * @return related resource uuids
     */
    @Select(RELATED_RESOURCE_SQL)
    List<String> queryRelatedResourceUuids(
            @Param("ids") List<String> parentUuids, @Param("exs") List<String> excludeResourceUuids);

    /**
     * query exist resource uuid list
     *
     * @param uuidList resource uuid list
     * @return exist resource uuid list
     */
    @Select(EXIST_RESOURCE_SQL)
    List<String> queryExistResourceUuidList(@Param("ids") List<String> uuidList);

    /**
     * query resource uuids
     *
     * @param page page
     * @param queryWrapper query wrapper
     * @return cursor
     */
    @Select(RESOURCE_UUID_SQL)
    IPage<String> queryResourceUuids(IPage<ProtectedResourcePo> page,
        @Param(Constants.WRAPPER) Wrapper<ProtectedResourcePo> queryWrapper);

    /**
     * select root uuid
     *
     * @param uuid uuid
     * @return root uuid
     */
    @Select(ROOT_UUID_SQL)
    String queryRootUuid(@Param("id") String uuid);

    /**
     * 查询资源所有依赖关系
     *
     * @param uuids resource uuids
     * @return 依赖关系
     */
    @Select(DEPENDENCY_RELATION_SQL)
    List<ResourceDependencyRelation> queryResourceDependencyRelation(@Param("ids") List<String> uuids);

    /**
     * 根据sourceType查询存在的uuid
     *
     * @param uuidList uuidList
     * @param sourceType sourceType
     * @return uuidList
     */
    @Select(RESOURCE_UUID_BY_SOURCE_TYPE_SQL)
    List<String> queryExistResourceUuidBySourceType(@Param("ids") List<String> uuidList,
        @Param("sourceType") String sourceType);

    /**
     * 根据rootUuid查询存在的uuid（安全一体机专用）
     *
     * @param rootUuid rootUuid
     * @return uuidList
     */
    @Select(RESOURCE_UUID_BY_ROOT_UUID_CYBER_ENGINE_SQL)
    List<String> queryResourceUuidsByRootUuidCyberEngine(@Param("rootUuid") String rootUuid);

    /**
     * 更新插件资源userId为空
     *
     * @param parentUuid parentUuid
     */
    @Update("update resources set USER_ID = null, AUTHORIZED_USER = null where PARENT_UUID = #{parentUuid} and TYPE = "
        + "'Plugin'")
    void updatePluginResourceUserId(@Param("parentUuid") String parentUuid);

    /**
     * 获取所有已存在资源的子类型
     *
     * @return 所有子类型的列表
     */
    List<String> getAllSubTypeList();

    /**
     * 根据插件应用类型appLabel Type查询在线的代理主机信息
     *
     * @param appLabelType appLabelType
     * @return uuidList
     */
    @Select("select distinct "
        + "res.*, "
        + "env.endpoint as endpoint, "
        + "env.os_type as os_type, "
        + "env.port as port "
        + "from resources as res "
        + "left join res_extend_info as res_ext on res.uuid = res_ext.resource_id "
        + "left join environments as env on res.uuid = env.uuid "
        + "where res.TYPE = 'Host' "
        + "and res.SUB_TYPE = 'UBackupAgent' "
        + "and env.link_status = 1 "
        + "and res_ext.key = 'agent_applications' "
        + "and res_ext.value like CONCAT('%',#{appLabelType},'%');")
    List<ProtectedResource> queryOnlineAgentListByAppLabel(@Param("appLabelType") String appLabelType);
}
