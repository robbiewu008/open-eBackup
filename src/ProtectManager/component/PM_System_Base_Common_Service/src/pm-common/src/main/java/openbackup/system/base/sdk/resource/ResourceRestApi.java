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
package openbackup.system.base.sdk.resource;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.PageQueryRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.model.AddImportResourceParam;
import openbackup.system.base.sdk.resource.model.AddImportResourceResponse;
import openbackup.system.base.sdk.resource.model.ClusterConfig;
import openbackup.system.base.sdk.resource.model.FileSetEntity;
import openbackup.system.base.sdk.resource.model.HostInfo;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.sdk.resource.model.VCenter;
import openbackup.system.base.sdk.resource.model.VirtualResourceSchema;
import openbackup.system.base.sdk.resource.model.VmInfo;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;
import java.util.stream.Collectors;

/**
 * get protected resource
 *
 * @author l00557046
 * @since 2020-08-10
 */
@FeignClient(name = "ResourceService", url = "${pm-resource-manager.url}/v1",
    configuration = CommonFeignConfiguration.class)
public interface ResourceRestApi {
    /**
     * 获取文件集信息
     *
     * @param resourceId 资源ID
     * @return FileSetEntity
     */
    @ExterAttack
    @GetMapping("/internal/resource/{resource_id}")
    @ResponseBody
    FileSetEntity queryResource(@PathVariable("resource_id") String resourceId);

    /**
     * 获取资源信息
     *
     * @param resourceId resourceId
     * @return resource info
     * */
    @ExterAttack
    @GetMapping("/internal/resource/{resource_id}")
    @ResponseBody
    String queryResourceById(@PathVariable("resource_id") String resourceId);

    /**
     * 获取资源信息
     *
     * @param resourceId resourceId
     * @param clazz clazz
     * @return resource info
     * */
    default <T> T queryResourceById(String resourceId, Class<T> clazz) {
        return JSONObject.toBean(queryResourceById(resourceId), clazz);
    }

    /**
     * verify resource ownership
     *
     * @param userId user id
     * @param resourceUuidList resource uuid list
     */
    @GetMapping("/internal/resource/action/verify")
    @ResponseBody
    void verifyResourceOwnership(@RequestParam("user_id") String userId,
        @RequestParam("resource_uuid_list") List<String> resourceUuidList);

    /**
     * 获取文件集信息
     *
     * @param resourceId 资源ID
     * @return FileSetEntity
     */
    @ExterAttack
    @GetMapping("/internal/resource/{resource_id}")
    @ResponseBody
    VCenter queryVCenterInfo(@PathVariable("resource_id") String resourceId);

    /**
     * 获取文件集信息
     *
     * @param resourceId 资源ID
     * @return FileSetEntity
     */
    @ExterAttack
    @GetMapping("/internal/resource/{resource_id}")
    @ResponseBody
    VmInfo queryVMInfo(@PathVariable("resource_id") String resourceId);

    /**
     * 获取文件集信息
     *
     * @param resourceId 资源ID
     * @return FileSetEntity
     */
    @ExterAttack
    @GetMapping("/internal/resource/{resource_id}")
    @ResponseBody
    HostInfo queryHostInfo(@PathVariable("resource_id") String resourceId);

    /**
     * 获取集群drs_config配置 开启或关闭
     *
     * @param resourceId 集群计算资源ID
     * @return clusterConfig
     */
    @ExterAttack
    @GetMapping("/internal/compute-resources/clusters/{resource_id}/config")
    @ResponseBody
    ClusterConfig queryClusterConfig(@PathVariable("resource_id") String resourceId);

    /**
     * 获取虚拟机信息
     *
     * @param resourceId 资源ID
     * @return virtualResourceSchema
     */
    @ExterAttack
    @GetMapping("/internal/resource/{resource_id}")
    @ResponseBody
    VirtualResourceSchema queryVirtualResource(@PathVariable("resource_id") String resourceId);

    /**
     * 获取agent信息
     *
     * @return HostInfo
     */
    @ExterAttack
    @GetMapping("/internal/vmbackupagent/")
    @ResponseBody
    HostInfo queryVMwareAgent();

    /**
     * 查询虚拟机列表
     *
     * @param page page
     * @param size size
     * @param orders orders
     * @param condition condition
     * @return virtualResourceSchema
     */
    @ExterAttack
    @GetMapping("/internal/virtual-resource")
    @ResponseBody
    BasePage<VirtualResourceSchema> queryVMResources(@RequestParam("page_no") int page,
        @RequestParam("page_size") int size, @RequestParam("conditions") String condition,
        @RequestParam("orders") List<String> orders);

    /**
     * 查询文件集列表
     *
     * @param page page
     * @param size size
     * @param orders orders
     * @param condition condition
     * @return virtualResourceSchema
     */
    @ExterAttack
    @GetMapping("/internal/resource")
    @ResponseBody
    BasePage<ResourceEntity> queryResource(@RequestParam("page_no") int page,
        @RequestParam("page_size") int size, @RequestParam("conditions") String condition,
        @RequestParam("orders") List<String> orders);

    /**
     * query environment sub resource application version
     *
     * @param environmentResource item
     * @param applicationSubType application sub type
     * @return version list
     */
    @ExterAttack
    default List<String> queryEnvironmentSubResourceApplicationVersion(ResourceEntity environmentResource,
        String applicationSubType) {
        List<ResourceEntity> resources = PageQueryRestApi.get(this::queryResource)
                .queryAll(new JSONObject().set("type", ResourceTypeEnum.APPLICATION.getType())
                        .set("sub_type", applicationSubType)
                        .set("parent_uuid", environmentResource.getUuid()))
                .getItems();
        return resources.stream()
                .filter(resource -> applicationSubType.equals(resource.getSubType()))
                .map(ResourceEntity::getVersion)
                .collect(Collectors.toList());
    }

    /**
     * 重置Resource user id
     *
     * @param userId user id
     */
    @PutMapping("/internal/resource/action/revoke/{user_id}")
    void revokeResourceUserId(@PathVariable("user_id") String userId);

    /**
     * add import resource
     *
     * @param param param
     * @return add import resource response
     * */
    @PostMapping("/internal/resources/action/import")
    @ResponseBody
    AddImportResourceResponse addImportResource(AddImportResourceParam param);
}
