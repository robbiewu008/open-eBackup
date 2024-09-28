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
package openbackup.access.framework.resource.service;

import openbackup.access.framework.resource.persistence.model.ProtectedAgentExtendPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourceExtendInfoPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.access.framework.resource.persistence.model.ResourceRepositoryQueryParams;
import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceGroupResult;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteParams;
import openbackup.data.protection.access.provider.sdk.resource.VstoreResourceQueryParam;
import openbackup.system.base.common.enums.ConsistentStatusEnum;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.query.Pagination;
import openbackup.system.base.sdk.copy.model.BasePage;

import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * 受保护资源存储库，提供受保护资源持久化标准接口，该接口的定义和具体的持久化框架无关
 *
 */
public interface ProtectedResourceRepository {
    /**
     * create protected resource bo
     *
     * @param resource resource
     * @return resource uuid
     */
    String create(ProtectedResource resource);

    /**
     * update resource
     *
     * @param resource resource
     * @param isOverwrite overwrite flag
     */
    void update(ProtectedResource resource, boolean isOverwrite);

    /**
     * updateLinkStatus
     *
     * @param uuid uuid
     * @param linkStatus linkStatus
     */
    void updateLinkStatus(String uuid, String linkStatus);

    /**
     * delete resource by id
     *
     * @param params 删除参数
     * @return 已删除的资源ID
     */
    List<String> delete(ResourceDeleteParams params);

    /**
     * delete protected environment in cyberEngine
     *
     * @param environmentId 环境uuid
     * @return 已删除的资源ID
     */
    List<String> deleteCyberEngineEnvironment(String environmentId);

    /**
     * page query
     *
     * @param page page
     * @param size size
     * @param conditions conditions
     * @param orders orders
     * @return page data
     */
    BasePage<ProtectedResourcePo> query(int page, int size, Map<String, Object> conditions, String... orders);

    /**
     * page query
     *
     * @param params ResourceRepositoryQueryParams
     * @return page data
     */
    BasePage<ProtectedResourcePo> query(ResourceRepositoryQueryParams params);

    /**
     * query page data group by key corresponding value
     *
     * @param param param
     * @return page data
     */
    BasePage<ProtectedResourceGroupResult> groupQuery(VstoreResourceQueryParam param);

    /**
     * query related resource uuids
     *
     * @param parentUuids parent uuids
     * @param excludeResourceUuids exclude resource uuids
     * @return related resource uuids
     */
    Set<String> queryRelatedResourceUuids(List<String> parentUuids, String... excludeResourceUuids);

    /**
     * query resource uuids
     *
     * @param pagination pagination
     * @return result
     */
    BasePage<String> queryResourceUuids(Pagination<JSONObject> pagination);

    /**
     * query dependency resources
     *
     * @param key dependency key
     * @param uuids query uuids
     * @return dependency resources
     */
    List<ProtectedResourcePo> queryDependencyResources(String key, List<String> uuids);

    /**
     * 查询用户名下资源(无session模式)
     *
     * @param userId 用户id
     * @return 资源列表
     */
    List<ProtectedResourcePo> queryResourcesByUserId(String userId);

    /**
     * 根据条件查询agent主机列表
     *
     * @param map 查询条件
     * @return 资源列表
     */
    List<ProtectedResourcePo> queryAgentResourceList(Map<String, Object> map);

    /**
     * 更新agent多租户共享
     *
     * @param uuid 资源uuid
     * @param isShared 是否开启共享
     */
    void updateAgentShared(String uuid, Boolean isShared);

    /**
     * 根据条件查询agent主机个数
     *
     * @param map 查询条件
     * @return 资源列表个数
     */
    int queryAgentResourceCount(Map<String, Object> map);

    /**
     * 根据sourceType查询存在的uuid
     *
     * @param uuidList uuidList
     * @param sourceType sourceType
     * @return uuidList
     */
    List<String> queryExistResourceUuidBySourceType(List<String> uuidList, String sourceType);

    /**
     * 根据parentUuid查询存在的uuid（安全一体机专用）
     *
     * @param parentUuid parentUuid
     * @return uuidList
     */
    List<String> queryResourceUuidsByRootUuidCyberEngine(String parentUuid);

    /**
     * 根据uuid集合查询受保护的资源
     *
     * @param uuids uuid集合
     * @return 受保护资源集合
     */
    List<ProtectedObjectPo> queryProtectedObject(List<String> uuids);

    /**
     * 查询指定数据状态的保护对象的数量
     *
     * @param status 数据状态
     * @return 指定数据状态的保护对象的数量
     */
    Integer queryProtectObjectCountByConsistentStatus(ConsistentStatusEnum status);

    /**
     * 查询指定数据状态的保护对象的 Id
     *
     * @param status 数据状态
     * @return 指定数据状态的保护对象的 Id 列表
     */
    List<String> queryProtectObjectIdListByConsistentStatus(ConsistentStatusEnum status);

    /**
     * 查询指定 Id 的保护对象
     *
     * @param uuid 保护对象 UUID
     * @return 保护对象
     */
    ProtectedObjectPo queryProtectObjectById(String uuid);

    /**
     * 根据uuid集合查询资源
     *
     * @param uuids uuid集合
     * @return 资源集合
     */
    List<ProtectedResourcePo> queryProtectedResource(List<String> uuids);

    /**
     * 更新所有保护对象的数据状态
     *
     * @param status 数据状态
     */
    void updateAllProtectObjectConsistentStatus(ConsistentStatusEnum status);

    /**
     * 更新指定 Id 的保护对象的检测状态和结果
     *
     * @param uuid 保护对象 UUID
     * @param status 保护对象的数据状态
     * @param consistentResults 保护对象的一致性检测结果
     */
    void updateProtectObjectConsistentById(String uuid, ConsistentStatusEnum status, String consistentResults);

    /**
     * 根据 root uuid 查询所有资源的uuid
     *
     * @param path path
     * @param rootUuid rootUuuid
     * @return uuid set
     */
    List<ProtectedResource> queryAllResourceIdsByPathAndRootUuid(String path, String rootUuid);

    /**
     * 更新dmeips
     *
     * @param uuid 资源uuid
     * @param dmeDomains dme的ip
     */
    void updateResExtendDomain(String uuid, String dmeDomains);

    /**
     * 更新agent CPU和内存占用率
     *
     * @param curAgentExtendInfo curAgentExtendInfo
     */
    void updateResExtendCpuAndMemRate(ProtectedAgentExtendPo curAgentExtendInfo);

    /**
     * 根据uuid查询agent拓展表信息
     *
     * @param uuid uuid
     * @return ProtectedAgentExtendPo
     */
    ProtectedAgentExtendPo queryProtectedAgentExtendByUuid(String uuid);

    /**
     * 查询扩展信息
     *
     * @param resUuid 资源ID
     * @param key 作为查询条件的key
     * @return 资源扩展信息 res_extend相关
     */
    List<ProtectedResourceExtendInfoPo> queryExtendInfoListByResourceIdAndKey(String resUuid, String key);

    /**
     * 保存或更新扩展信息
     *
     * @param uuid 资源id
     * @param value value值
     * @param keyName key
     */
    void saveOrUpdate(String uuid, String value, String keyName);

    /**
     * 修改resource userId
     *
     * @param uuid 资源id
     * @param userId 用户id
     * @param authorizedUserName 所属用户名称
     */
    void updateUserId(String uuid, String userId, String authorizedUserName);

    /**
     * 根据key和资源id删除资源拓展信息
     *
     * @param resourceId resourceId
     * @param key key
     */
    void deleteProtectResourceExtendInfoByResourceId(String resourceId, String key);

    /**
     * 对需要使用老private签名的agent添加标记
     */
    void legoHostSighWithOldPrivateKey();
}
