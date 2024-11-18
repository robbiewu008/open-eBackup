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

import static openbackup.access.framework.resource.util.ResourceUtil.convertStorageType;

import com.huawei.oceanprotect.system.base.label.dao.LabelResourceServiceDao;

import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.access.framework.resource.persistence.model.ResourceRepositoryQueryParams;
import openbackup.access.framework.resource.vo.ProtectedResourceLoggingVo;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.CyberEngineResourceService;
import openbackup.data.protection.access.provider.sdk.resource.FileSystemInfo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.StorageInfo;
import openbackup.data.protection.access.provider.sdk.resource.TenantInfo;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.security.callee.CalleeMethod;
import openbackup.system.base.security.callee.CalleeMethods;
import openbackup.system.base.util.MessageTemplate;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.context.request.RequestAttributes;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

import javax.servlet.http.HttpServletRequest;

/**
 * 功能描述 安全一体机受保护资源服务类
 *
 */
@Service
@Slf4j
@CalleeMethods(name = "cyber_resource_service", value = {
    @CalleeMethod(name = "getBasicResourceInfoById")
})
public class CyberEngineResourceServiceImpl implements CyberEngineResourceService {
    // 文件系统ID
    private static final String FILE_SYSTEM_ID = "fileSystemId";

    // 文件系统名称
    private static final String FILE_SYSTEM_NAME = "fileSystemName";

    // 文件系统子类型
    private static final String FILE_SUB_TYPE = "fileSubType";

    // 租户名称
    private static final String TENANT_NAME = "tenantName";

    // 租户ID
    private static final String TENANT_ID = "tenantId";

    // 用户ID
    private static final String ROOT_UUID = "rootUuid";

    // 类型字段
    private static final String TYPE = "type";

    // 子类型字段
    private static final String SUB_TYPE = "subType";

    // 设备信息
    private static final String STORAGE_EQUIPMENT = "StorageEquipment";

    private final ProtectedResourceRepository repository;

    private MessageTemplate<String> messageTemplate;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private TokenVerificationService tokenVerificationService;

    @Autowired
    private LabelResourceServiceDao labelResourceServiceDao;

    /**
     * constructor
     *
     * @param repository 仓库类
     */
    public CyberEngineResourceServiceImpl(ProtectedResourceRepository repository) {
        this.repository = repository;
    }

    @Autowired
    public void setMessageTemplate(MessageTemplate<String> messageTemplate) {
        this.messageTemplate = messageTemplate;
    }

    /**
     * 批量查询所有租户信息
     *
     * @param pageNo 页面编号
     * @param pageSize 页面大小
     * @return 租户信息
     */
    public PageListResponse<TenantInfo> listAllTenants(int pageNo, int pageSize) {
        log.info("CyberEngine start to get all tenants, pageNo {}, pageSize {}", pageNo, pageSize);
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(TYPE, ResourceTypeEnum.STORAGE.getType());
        conditions.put(SUB_TYPE, ResourceTypeEnum.TENANT.getType());
        return getTenantInfoPageListResponse(pageNo, pageSize, conditions);
    }

    /**
     * 根据设备Id查询当前设备下所有租户
     *
     * @param deviceId 设备Id
     * @return 租户信息
     */
    public PageListResponse<TenantInfo> listAllTenantsByDeviceId(String deviceId) {
        log.info("CyberEngine start to get all tenants by deviceId {}", deviceId);
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(TYPE, ResourceTypeEnum.STORAGE.getType());
        conditions.put(SUB_TYPE, ResourceTypeEnum.TENANT.getType());
        conditions.put(ROOT_UUID, deviceId);
        BasePage<ProtectedResource> data = repository.query(
            new ResourceRepositoryQueryParams(false, 0, LegoNumberConstant.THROUND_TWENTY_FOUR, conditions,
                new String[0])).map(ProtectedResourcePo::toProtectedResource);
        return getTenantInfoPageListResponse(data);
    }

    private PageListResponse<TenantInfo> getTenantInfoPageListResponse(BasePage<ProtectedResource> data) {
        if (data.getTotal() == 0) {
            return new PageListResponse<>();
        }
        List<TenantInfo> tenantInfos = data.getItems()
            .stream()
            .map(resource -> buildTenantInfo(resource))
            .collect(Collectors.toList());
        return new PageListResponse<>(tenantInfos.size(), tenantInfos);
    }

    /**
     * 使用文件系统ID查询租户信息
     *
     * @param pageNo 页面编号
     * @param pageSize 页面大小
     * @param resourceId 文件系统ID
     * @return 租户信息
     */
    public PageListResponse<TenantInfo> listTenantByResourceId(int pageNo, int pageSize, String resourceId) {
        log.info("CyberEngine start to get tenant by resourceId, pageNo {}, pageSize {}, resourceId {}", pageNo,
            pageSize, resourceId);

        // 1、根据文件系统ID查询租户ID
        String tenantId = getTenantIdByResource(pageNo, pageSize, resourceId);
        if (StringUtils.isBlank(tenantId)) {
            return new PageListResponse<>();
        }

        // 2、根据租户ID查询租户信息
        Map<String, Object> queryInfos = new HashMap<>();
        queryInfos.put(TYPE, ResourceTypeEnum.STORAGE.getType());
        queryInfos.put(SUB_TYPE, ResourceTypeEnum.TENANT.getType());
        queryInfos.put(TENANT_ID, tenantId);
        return getTenantInfoPageListResponse(pageNo, pageSize, queryInfos);
    }

    private PageListResponse<TenantInfo> getTenantInfoPageListResponse(int pageNo, int pageSize,
        Map<String, Object> queryInfos) {
        BasePage<ProtectedResource> tenants = repository.query(
            new ResourceRepositoryQueryParams(false, pageNo, pageSize, queryInfos, new String[0]))
            .map(ProtectedResourcePo::toProtectedResource);
        return getTenantInfoPageListResponse(tenants);
    }

    private String getTenantIdByResource(int pageNo, int pageSize, String resourceId) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(TYPE, ResourceTypeEnum.STORAGE.getType());
        conditions.put(SUB_TYPE, ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType());
        conditions.put(FILE_SYSTEM_ID, resourceId);
        BasePage<ProtectedResource> fileInfos = repository.query(
            new ResourceRepositoryQueryParams(false, pageNo, pageSize, conditions, new String[0]))
            .map(ProtectedResourcePo::toProtectedResource);
        if (fileInfos.getTotal() == 0) {
            return "";
        }
        List<FileSystemInfo> fileSystemInfos = fileInfos.getItems()
            .stream()
            .map(resource -> buildFileSystemInfo(resource))
            .collect(Collectors.toList());
        FileSystemInfo fileSystemInfo = fileSystemInfos.get(0);
        return fileSystemInfo.getTenantId();
    }

    private TenantInfo buildTenantInfo(ProtectedResource protectedResource) {
        Map<String, String> extendInfo = protectedResource.getExtendInfo();
        return new TenantInfo(extendInfo.get(TENANT_ID), extendInfo.get(TENANT_NAME), protectedResource.getParentUuid(),
            protectedResource.getParentName());
    }

    private FileSystemInfo buildFileSystemInfo(ProtectedResource protectedResource) {
        Map<String, String> extendInfo = protectedResource.getExtendInfo();
        FileSystemInfo fileSystemInfo = new FileSystemInfo();
        fileSystemInfo.setDeviceId(protectedResource.getRootUuid());
        fileSystemInfo.setTenantId(extendInfo.get(TENANT_ID));
        fileSystemInfo.setFileUuid(protectedResource.getUuid());
        fileSystemInfo.setFileId(extendInfo.get(FILE_SYSTEM_ID));
        fileSystemInfo.setFileName(extendInfo.get(FILE_SYSTEM_NAME));
        fileSystemInfo.setSubType(extendInfo.get(FILE_SUB_TYPE));
        return fileSystemInfo;
    }

    /**
     * 批量查设备信息
     *
     * @param deviceId 设备Id
     * @param tenantId 租户Id
     * @return 设备信息
     */
    public StorageInfo listStorageInfo(String deviceId, String tenantId) {
        log.info("CyberEngine start to get storageInfo, deviceId {}", deviceId);
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(TYPE, ResourceTypeEnum.STORAGE.getType());
        conditions.put(SUB_TYPE, ResourceTypeEnum.TENANT.getType());
        conditions.put(TENANT_ID, tenantId);
        conditions.put(ROOT_UUID, deviceId);
        BasePage<ProtectedResource> fileInfos = repository.query(
            new ResourceRepositoryQueryParams(false, 0, LegoNumberConstant.TEN, conditions, new String[0]))
            .map(ProtectedResourcePo::toProtectedResource);
        if (fileInfos.getTotal() == 0) {
            return new StorageInfo();
        }
        if (!StringUtils.equals(fileInfos.getItems().get(0).getRootUuid(), deviceId)) {
            log.info("no deviceId and tenantId exist");
            return new StorageInfo();
        }
        log.info("deviceId and tenantId exist {}", deviceId);
        Map<String, Object> condition = new HashMap<>();
        condition.put(TYPE, STORAGE_EQUIPMENT);
        condition.put(ROOT_UUID, deviceId);
        BasePage<ProtectedResource> fileInfo = repository.query(
            new ResourceRepositoryQueryParams(false, 0, LegoNumberConstant.TEN, condition, new String[0]))
            .map(ProtectedResourcePo::toProtectedResource);
        if (fileInfo.getTotal() == 0) {
            return new StorageInfo();
        }
        return buildStorageInfo(fileInfo.getItems().get(0), tenantId);
    }

    /**
     * 批量查文件系统信息
     *
     * @param pageNo 页面编号
     * @param pageSize 页面大小
     * @param tenantId 租户Id
     * @param resourceId 文件系统Id
     * @return 文件系统信息
     */
    public PageListResponse<FileSystemInfo> listFileSystems(int pageNo, int pageSize, String tenantId,
        String resourceId) {
        log.info("CyberEngine start to get file systems, pageNo {}, pageSize {}, tenantId {}", pageNo, pageSize,
            tenantId);
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(TYPE, ResourceTypeEnum.STORAGE.getType());
        conditions.put(SUB_TYPE, ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType());
        conditions.put(FILE_SYSTEM_ID, resourceId);
        conditions.put(TENANT_ID, tenantId);
        BasePage<ProtectedResource> fileInfos = repository.query(
            new ResourceRepositoryQueryParams(false, pageNo, pageSize, conditions, new String[0]))
            .map(ProtectedResourcePo::toProtectedResource);
        if (fileInfos.getTotal() == 0) {
            return new PageListResponse<>();
        }
        List<FileSystemInfo> fileSystemInfos = fileInfos.getItems()
            .stream()
            .map(resource -> buildFileSystemInfo(resource))
            .collect(Collectors.toList());
        return new PageListResponse<>(fileSystemInfos.size(), fileSystemInfos);
    }

    @Override
    public void deleteEnvironment(String environmentId) {
        List<String> deletedResourceUuids = repository.deleteCyberEngineEnvironment(environmentId);
        labelResourceServiceDao.deleteByResourceObjectIdsAndLabelIds(deletedResourceUuids.stream()
            .distinct()
            .collect(Collectors.toList()), StringUtils.EMPTY);
        for (String deletedResourceUuid : new HashSet<>(deletedResourceUuids)) {
            log.info("resource({}) deleted", deletedResourceUuid);
            messageTemplate.send("resource.deleted", new JSONObject().set("resource_id", deletedResourceUuid));
        }
    }

    private StorageInfo buildStorageInfo(ProtectedResource protectedResource, String tenantId) {
        Authentication auth = protectedResource.getAuth();
        StorageInfo storageInfo = new StorageInfo();
        storageInfo.setUserName(auth.getAuthKey());
        storageInfo.setPassword(auth.getAuthPwd());
        storageInfo.setEndpoint(protectedResource.getPath());
        storageInfo.setPort(protectedResource.getPort());
        storageInfo.setType(protectedResource.getSubType());
        storageInfo.setDeviceId(protectedResource.getUuid());
        storageInfo.setTenantId(tenantId);
        storageInfo.setUserId(protectedResource.getUserId());
        return storageInfo;
    }

    /**
     * 查询资源,组装logging所需信息-eventInfo属性 -安全一体机
     * 适用场景：查询资源的基本属性，不再额外查询资源的dependency依赖信息
     *
     * @param copyId 副本ID
     * @return 返回loggingVo
     */
    public ProtectedResourceLoggingVo getBasicResourceInfoById(String copyId) {
        String resourceId = copyRestApi.queryCopyByID(copyId).getResourceId();
        // 切面order问题loggingAspect优先于operationLogAspect，token_bo没有设置
        RequestAttributes requestAttributes = RequestContextHolder.getRequestAttributes();
        if (requestAttributes instanceof ServletRequestAttributes) {
            HttpServletRequest request = ((ServletRequestAttributes) requestAttributes).getRequest();
            request.setAttribute("token_bo", tokenVerificationService.parsingTokenFromRequest());
        }
        Optional<ProtectedResource> protectedResource = resourceService.getBasicResourceById(resourceId);
        ProtectedResourceLoggingVo res = new ProtectedResourceLoggingVo();
        if (!protectedResource.isPresent()) {
            return res;
        }
        ProtectedResource resource = protectedResource.get();
        res.setResourceId(resource.getUuid());
        res.setResourceName(resource.getName());
        res.setTenantName(resource.getParentName());
        res.setTenantId(resource.getParentUuid());
        res.setStorageUUID(resource.getRootUuid());
        String path = resource.getPath();
        String[] pathArr = path.split("/");
        res.setStorageType(convertStorageType(pathArr[0]));
        res.setStorageName(pathArr[1]);
        return res;
    }
}