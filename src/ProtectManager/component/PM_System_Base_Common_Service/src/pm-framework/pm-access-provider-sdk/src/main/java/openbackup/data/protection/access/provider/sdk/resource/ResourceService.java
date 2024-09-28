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
package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.backup.NextBackupModifyReq;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.model.ResourceScanDto;
import openbackup.data.protection.access.provider.sdk.resource.model.ResourceUpsertRes;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.function.Consumer;

/**
 * 资源服务接口定义
 *
 */
public interface ResourceService {
    /**
     * 创建资源
     *
     * @param resources 受保护资源
     * @return resource uuid list
     */
    default String[] create(ProtectedResource[] resources) {
        return create(resources, false);
    }

    /**
     * 创建资源
     *
     * @param resources 受保护资源
     * @param shouldExecuteAutoScan 是否执行自动扫描
     * @return resource uuid list
     */
    String[] create(ProtectedResource[] resources, boolean shouldExecuteAutoScan);

    /**
     * 检查资源, 返回检查结果
     *
     * @param resource 受保护资源
     * @return 检查结果
     */
    default ActionResult[] check(ProtectedResource resource) {
        return check(resource, false);
    }

    /**
     * 检查资源, 对检查结果做检查
     *
     * @param resource 受保护资源
     * @param isCheckResult 是否对检查结果做检查，若结果不成功，则抛出异常
     * @return 检查结果
     */
    ActionResult[] check(ProtectedResource resource, boolean isCheckResult);

    /**
     * update resources
     *
     * @param isOverwrite overwrite flag
     * @param resources resources
     */
    void update(boolean isOverwrite, ProtectedResource[] resources);

    /**
     * update resources
     *
     * @param resources resources
     */
    void update(ProtectedResource[] resources);

    /**
     * updateLinkStatus
     *
     * @param uuid uuid
     * @param linkStatus linkStatus
     */
    void updateLinkStatus(String uuid, String linkStatus);

    /**
     * update source type of protected resource by uuid list
     *
     * @param uuids uuids
     * @param sourceType source type
     */
    void updateSourceType(List<String> uuids, String sourceType);

    /**
     * update resource's sub resources
     *
     * @param uuids current resource's uuid
     * @param updateEntry update kv
     */
    void updateSubResource(List<String> uuids, Map<String, Object> updateEntry);

    /**
     *  直接更新资源到数据库
     *  适用场景：
     *  适用于直接将资源的某些属性同步到数据库中而不用运行入库前其他逻辑的更新情况，
     *  只需要保证待更新的属性能够入库成功。
     *
     * @param resources 待更新的资源
     */
    void updateSourceDirectly(List<ProtectedResource> resources);

    /**
     * update or insert resources
     *
     * @param isOverwrite overwrite flag
     * @param resources resources
     * @return resource uuids
     */
    ResourceUpsertRes upsert(boolean isOverwrite, ProtectedResource[] resources);

    /**
     * update or insert resources
     *
     * @param resources resources
     * @return resource uuids
     */
    default ResourceUpsertRes upsert(ProtectedResource[] resources) {
        return upsert(false, resources);
    }

    /**
     * merge auth param from source to target
     *
     * @param authentications authentications
     * @return new auth object
     */
    Authentication mergeAuthentication(Authentication[] authentications);

    /**
     * merge protected resource
     *
     * @param protectedResource protectedResource
     * @return resource
     */
    ProtectedResource mergeProtectedResource(ProtectedResource protectedResource);

    /**
     * replenish Environment
     *
     * @param resource resource
     * @return resource
     */
    ProtectedResource replenishEnvironment(ProtectedResource resource);

    /**
     * 创建受保护资源扫描任务
     *
     * @param resId env id
     * @param userId user id
     */
    void createProtectedResourceScanTask(String resId, String userId);

    /**
     * 扫描受保护资源
     *
     * @param resource resource
     * @param isStrict strict mode
     * @param afterScan afterScan
     * @return 扫描完成后，新增资源的UUID列表
     */
    List<String> scanProtectedResource(
            ProtectedResource resource, boolean isStrict, Consumer<ResourceScanDto> afterScan);

    /**
     * 扫描受保护资源
     *
     * @param resource resource
     * @param isStrict strict mode
     * @return 扫描完成后，新增资源的UUID列表
     */
    default List<String> scanProtectedResource(ProtectedResource resource, boolean isStrict) {
        return scanProtectedResource(resource, isStrict, null);
    }

    /**
     * 扫描受保护资源
     *
     * @param resource resource
     * @return 扫描完成后，新增资源的UUID列表
     */
    default List<String> scanProtectedResource(ProtectedResource resource) {
        return scanProtectedResource(resource, true, null);
    }

    /**
     * 删除资源
     *
     * @param resources 资源ID
     */
    default void delete(String[] resources) {
        delete(new ResourceDeleteParams(false, true, resources));
    }

    /**
     * 删除资源
     *
     * @param params 删除参数
     */
    void delete(ResourceDeleteParams params);

    /**
     * page query for protected resource
     *
     * @param page page
     * @param size size
     * @param conditions page query conditions
     * @param orders orders
     * @return page data
     */
    default PageListResponse<ProtectedResource> query(
            int page, int size, Map<String, Object> conditions, String... orders) {
        return query(true, page, size, conditions, orders);
    }

    /**
     * 查询 条件下所有资源信息
     *
     * @param conditions 条件
     * @return 所有资源
     */
    default List<ProtectedResource> queryAllResources(Map<String, Object> conditions) {
        List<ProtectedResource> resourceList = new ArrayList<>();
        PageListResponse<ProtectedResource> basePage;
        int count = 0;
        do {
            basePage = query(count, 100, conditions, "");
            resourceList.addAll(basePage.getRecords());
            count++;
        } while (basePage.getRecords().size() == 100);
        return resourceList;
    }

    /**
     * page query for protected resource
     *
     * @param shouldDecrypt decrypt
     * @param page page
     * @param size size
     * @param conditions page query conditions
     * @param orders orders
     * @return page data
     */
    PageListResponse<ProtectedResource> query(
            boolean shouldDecrypt, int page, int size, Map<String, Object> conditions, String... orders);

    /**
     * page query for basic protected resource without loadEnvironment
     *
     * @param shouldDecrypt decrypt
     * @param page page
     * @param size size
     * @param conditions page query conditions
     * @param orders orders
     * @return page data
     */
    PageListResponse<ProtectedResource> basicQuery(
            boolean shouldDecrypt, int page, int size, Map<String, Object> conditions, String... orders);

    /**
     * page query for protected resource
     *
     * @param context query context
     * @return page data
     */
    PageListResponse<ProtectedResource> query(ResourceQueryParams context);

    /**
     * page query for protected resource
     *
     * @param param 查询参数
     * @return page data
     */
    PageListResponse<ProtectedResourceGroupResult> groupQueryByExtendInfo(VstoreResourceQueryParam param);

    /**
     * 查询资源
     *
     * @param resourceId 资源ID
     * @return 返回受保护资源
     */
    default Optional<ProtectedResource> getResourceById(String resourceId) {
        return getResourceById(true, resourceId);
    }

    /**
     * 查询资源
     *
     * @param shouldDecrypt decrypt flag
     * @param resourceId 资源ID
     * @return 返回受保护资源
     */
    Optional<ProtectedResource> getResourceById(boolean shouldDecrypt, String resourceId);

    /**
     * 查询资源
     * 适用场景：查询资源的基本属性，不再额外查询资源的dependency依赖信息
     *
     * @param resourceId 资源ID
     * @return 返回受保护资源
     */
    default Optional<ProtectedResource> getBasicResourceById(String resourceId) {
        return getBasicResourceById(true, resourceId);
    }

    /**
     * 查询资源
     * 适用场景：查询资源的基本属性，不再额外查询资源的dependency依赖信息
     *
     * @param shouldDecrypt decrypt flag
     * @param resourceId 资源ID
     * @return 返回受保护资源
     */
    default Optional<ProtectedResource> getBasicResourceById(boolean shouldDecrypt, String resourceId) {
        return getBasicResourceById(shouldDecrypt, false, resourceId);
    }

    /**
     * 查询资源
     * 适用场景：查询资源的基本属性，不再额外查询资源的dependency依赖信息
     *
     * @param shouldDecrypt 解密true,否则false
     * @param shouldLoadEnvironment 是否加载环境信息
     * @param resourceId resourceId
     * @return 返回受保护资源
     */
    Optional<ProtectedResource> getBasicResourceById(
            boolean shouldDecrypt, boolean shouldLoadEnvironment, String resourceId);

    /**
     * query related resource uuid
     *
     * @param parentUuids parent resource uuids
     * @param excludeResourceUuids excluded resource uuids
     * @return related resource uuids
     */
    Set<String> queryRelatedResourceUuids(List<String> parentUuids, String[] excludeResourceUuids);

    /**
     * query dependency resources
     *
     * @param shouldDecrypt decrypt flag
     * @param key dependency key
     * @param uuids query uuids
     * @return dependency resources
     */
    List<ProtectedResource> queryDependencyResources(boolean shouldDecrypt, String key, List<String> uuids);

    /**
     * 查询用户名下资源，不校验当前登录用户，根据入参决定
     *
     * @param userId 用户id
     * @return 资源列表
     */
    List<ProtectedResource> queryResourcesByUserId(String userId);

    /**
     * 查询主机列表
     *
     * @param map 查询条件map
     * @return 资源列表
     */
    PageListResponse<ProtectedResource> queryAgentResourceList(Map<String, Object> map);

    /**
     * query related resource uuid
     *
     * @param parentUuid parent resource uuid
     * @param excludeResourceUuids excluded resource uuids
     * @return related resource uuids
     */
    default Set<String> queryRelatedResourceUuids(String parentUuid, String[] excludeResourceUuids) {
        return queryRelatedResourceUuids(Collections.singletonList(parentUuid), excludeResourceUuids);
    }

    /**
     * 补充资源的dependency信息
     * 使用场景：交给应用去决定是否执行补齐资源的dependency信息，该过程相对耗时
     *
     * @param resource 待补充dependency信息的资源
     * @return 返回受保护资源
     */
    Optional<ProtectedResource> queryDependency(ProtectedResource resource);

    /**
     * 检查主机是否受信
     *
     * @param endpoint endpoint
     * @return boolean
     */
    default boolean checkHostIfBeTrustedByEndpoint(String endpoint) {
        return checkHostIfBeTrustedByEndpoint(endpoint, false);
    }

    /**
     * 检查主机是否受信
     *
     * @param endpoint endpoint
     * @param isCheckWhenFalse isCheckWhenFalse 若为true，当不受信时抛出异常
     * @return boolean
     */
    boolean checkHostIfBeTrustedByEndpoint(String endpoint, boolean isCheckWhenFalse);

    /**
     * 检查主机资源是否守信
     *
     * 内置主机为true
     * 通用代理,根据扩展字段trustworthiness判断
     *
     * @param records 资源
     */
    void checkHostIfBeTrusted(List<ProtectedEnvironment> records);

    /**
     * 脱敏, 对资源及其dependency脱敏
     *
     * @param resource 资源
     */
    void desensitize(ProtectedResource resource);

    /**
     * 是否已经加入虚拟组
     *
     * @param resource 资源
     */
    void appendGroupInfo(ProtectedResource resource);

    /**
     * 检测资源扫描任务是否在运行中
     *
     * @param resId 环境/资源id
     * @return 检查结果  true-有运行中的任务；false-没有运行中的任务
     */
    boolean checkEnvScanTaskIsRunning(String resId);

    /**
     * 根据 root uuid 查询所有资源的uuid
     *
     * @param path path路径
     * @param rootUuid root uuid
     * @return uuid set
     */
    List<ProtectedResource> queryAllResourceIdsByPathAndRootUuid(String path, String rootUuid);

    /**
     * 填充依赖数据
     *
     * @param resource 待填充依赖的资源
     */
    void setResourceDependency(ProtectedResource resource);

    /**
     * 根据ip查host
     *
     * @param endpoint ip
     * @return host
     */
    ProtectedEnvironment queryHostByEndpoint(String endpoint);

    /**
     * 通过agent id集合，获取其存在扩展表中的LanFree配置项
     * lanFree配置项 可能为0（未配置）,1（已配置）等
     *
     * @param subType 资源类型
     * @param resourceIds agent id集合
     * @return <agentId, lanFree配置项>
     */
    Map<String, String> getLanFreeConfig(String subType, List<String> resourceIds);

    /**
     * 设置下次备份参数
     *
     * @param nextBackupModifyReq 下次备份参数
     */
    default void modifyNextBackup(NextBackupModifyReq nextBackupModifyReq) {
        modifyNextBackup(nextBackupModifyReq, true);
    }

    /**
     * 设置下次备份参数
     *
     * @param nextBackupModifyReq 下次备份参数
     * @param isStrict true：当资源不存在时抛出异常；false：当资源不存在时直接返回
     */
    void modifyNextBackup(NextBackupModifyReq nextBackupModifyReq, boolean isStrict);

    /**
     * 取消下次备份参数
     *
     * @param resourceId 资源id
     */
    void cleanNextBackup(String resourceId);

    /**
     * 查询下次备份的参数接口
     *
     * @param resourceId 资源id
     * @return ResourceExtendParams 下次备份的参数
     */
    NextBackupParams queryNextBackupTypeAndCause(String resourceId);

    /**
     * 通过agent id集合，通过配置项查找 开启了lanFree 的集群绑定关系
     *
     * @param subType     资源类型
     * @param resourceIds agent id集合
     * @return 集群esn
     */
    List<String> getRelationInLanFree(String subType, List<String> resourceIds);

    /**
     * updateUserId
     *
     * @param uuid uuid
     * @param userId userId
     * @param authorizedUserName authorizedUserName
     */
    void updateUserId(String uuid, String userId, String authorizedUserName);

    /**
     * 查询资源，使用parentUuid查询
     *
     * @param resourceId 父级资源的uuid
     * @return List<ProtectedResource>
     */
    List<ProtectedResource> getResourceByParentId(String resourceId);

    /**
     * 查询资源uuid，根据key和value查询
     *
     * @param key 扩展属性的key
     * @param value 扩展属性的value
     * @return 资源uuid
     */
    Optional<String> getResourceIdByExKeyValue(String key, String value);

    /**
     * 查询资源uuid，根据key和value查询
     *
     * @param key 扩展属性的key
     * @param value 扩展属性的value
     * @return 资源的uuids
     */
    List<String> getResourceIdsByExKeyValue(String key, String value);

    /**
     * 查询资源
     *
     * @param resourceId 资源ID
     * @return 返回受保护资源
     */
    default Optional<ProtectedResource> getResourceByIdIgnoreOwner(String resourceId) {
        return getResourceByIdIgnoreOwner(true, resourceId);
    }

    /**
     * 查询资源
     *
     * @param shouldDecrypt decrypt flag
     * @param resourceId 资源ID
     * @return 返回受保护资源
     */
    Optional<ProtectedResource> getResourceByIdIgnoreOwner(boolean shouldDecrypt, String resourceId);

    /**
     * 更新插件资源的user_id为空
     *
     * @param uuid 主机资源id
     */
    void updatePluginResourceUserId(String uuid);

    /**
     * 查询资源
     *
     * @param resourceId 资源ID
     * @return 当前资源是否支持恢复
     */
    String judgeResourceRestoreLevel(String resourceId);

    /**
     * 检查父资源是否支持恢复
     *
     * @param resourceId 资源ID
     */
    void verifyParentResourceRestoreLevel(String resourceId);

    /**
     * 根据用户id获取用户域下所有的资源id集合
     *
     * @param userId 用户id
     * @return 用户域下资源id集合
     */
    List<String> getDomainResourceIdListByUserId(String userId);

    /**
     * 根据资源id查询资源信息
     *
     * @param resourceIds 资源id
     * @return 资源列表
     */
    List<ProtectedResource> getResourceListByUuidList(List<String> resourceIds);

    /**
     * 根据资源子类型列表查询所有资源id
     *
     * @param subTypeList 子类型列表
     * @return 资源列表
     */
    List<ProtectedResource> getResourceListBySubTypeList(List<String> subTypeList);
}
