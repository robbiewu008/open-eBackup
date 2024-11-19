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
package openbackup.data.access.framework.copy.index.service.impl;

import com.huawei.oceanprotect.base.cluster.sdk.dto.ClusterRequestInfo;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.util.ClusterUriUtil;
import com.huawei.oceanprotect.base.cluster.sdk.util.IpUtil;
import com.huawei.oceanprotect.system.base.user.bo.DomainInfoBo;
import com.huawei.oceanprotect.system.base.user.common.enums.DomainTypeEnum;
import com.huawei.oceanprotect.system.base.user.service.DomainService;
import com.huawei.oceanprotect.system.base.user.service.UserService;

import com.fasterxml.jackson.databind.JsonNode;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dee.DeeUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.dee.model.DeleteCopyIndexRequest;
import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.access.framework.agent.ProtectAgentSelector;
import openbackup.data.access.framework.copy.index.service.ICopyIndexService;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.copy.mng.constant.CopyResourcePropertiesConstant;
import openbackup.data.access.framework.core.common.constants.CopyIndexConstants;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.constants.AgentKeyConstant;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.index.v2.CopyIndexProvider;
import openbackup.data.protection.access.provider.sdk.index.v2.CopyIndexTask;
import openbackup.data.protection.access.provider.sdk.index.v2.CreateIndexCopyInfo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JSONObjectCovert;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import openbackup.system.base.sdk.cluster.TargetClusterRestApi;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.UpdateCopyIndexStatusRequest;
import openbackup.system.base.sdk.protection.SlaRestApi;
import openbackup.system.base.sdk.protection.emuns.SlaPolicyTypeEnum;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.SlaBo;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.service.DeployTypeService;

import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;

import java.net.URI;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * 统一框架副本创建索引服务类
 *
 */
@Service
@Slf4j
public class UnifiedCopyIndexService implements ICopyIndexService {
    private final ProviderManager providerManager;

    private final ResourceService resourceService;

    private final DeeUnifiedRestApi deeUnifiedRestApi;

    private final CopyRestApi copyRestApi;

    private final SlaRestApi slaRestApi;

    private DefaultProtectAgentSelector defaultSelector;

    private CopyManagerService copyManagerService;

    private UserService userService;

    private TargetClusterRestApi targetClusterRestApi;

    private MemberClusterService memberClusterService;

    private CommonAgentService commonAgentService;

    private TaskRepositoryManager taskRepositoryManager;

    private DomainService domainService;

    private DeployTypeService deployTypeService;

    public UnifiedCopyIndexService(ProviderManager providerManager, DeeUnifiedRestApi deeUnifiedRestApi,
        ResourceService resourceService, CopyRestApi copyRestApi, SlaRestApi slaRestApi) {
        this.providerManager = providerManager;
        this.deeUnifiedRestApi = deeUnifiedRestApi;
        this.resourceService = resourceService;
        this.copyRestApi = copyRestApi;
        this.slaRestApi = slaRestApi;
    }

    @Autowired
    public void setCommonAgentService(CommonAgentService commonAgentService) {
        this.commonAgentService = commonAgentService;
    }

    @Autowired
    public void setUserService(final UserService userService) {
        this.userService = userService;
    }

    @Autowired
    public void setCopyManagerService(CopyManagerService copyManagerService) {
        this.copyManagerService = copyManagerService;
    }

    @Autowired
    @Qualifier("memberClusterApiWithDmaProxyManagePort")
    public void setTargetClusterRestApi(TargetClusterRestApi targetClusterRestApi) {
        this.targetClusterRestApi = targetClusterRestApi;
    }

    @Autowired
    public void setMemberClusterService(MemberClusterService memberClusterService) {
        this.memberClusterService = memberClusterService;
    }

    @Autowired
    public void setDefaultSelector(DefaultProtectAgentSelector defaultSelector) {
        this.defaultSelector = defaultSelector;
    }

    @Autowired
    public void setTaskRepositoryManager(TaskRepositoryManager taskRepositoryManager) {
        this.taskRepositoryManager = taskRepositoryManager;
    }

    @Autowired
    public void setDomainService(DomainService domainService) {
        this.domainService = domainService;
    }

    @Autowired
    public void setDeployTypeService(DeployTypeService deployTypeService) {
        this.deployTypeService = deployTypeService;
    }

    @Override
    public CopyIndexTask createIndexTask(CopyBo copy, String requestId, String indexedMode) {
        // 判断是否支持和是否开启索引
        if (!isSupportIndex(copy)) {
            log.info("Resource {} is not support(UnifiedCopyIndexService). copy id is {}", copy.getResourceSubType(),
                copy.getUuid());
            return new CopyIndexTask();
        }
        if (!isIndex(copy, indexedMode)) {
            log.info("Resource {} do not open index. copy id is {}", copy.getResourceSubType(), copy.getUuid());
            return new CopyIndexTask();
        }
        CopyIndexTask indexTask = buildIndexTask(copy, requestId, indexedMode);
        copyRestApi.updateCopyIndexStatus(copy.getUuid(), CopyIndexStatus.INDEXING.getIndexStaus(), "");
        log.info("Update copy:{} index status: {} success.", copy.getUuid(), CopyIndexStatus.INDEXING.getIndexStaus());
        try {
            deeUnifiedRestApi.createIndexTask(indexTask);
            log.info("Send dee unified copy index task success, copy id is {}", copy.getUuid());
        } catch (Exception e) {
            log.error("Invoke dee creat index interface failed. copy id is {}", copy.getUuid(),
                ExceptionUtil.getErrorMessage(e));
            String errorCode = CopyIndexStatus.INDEX_RESPONSE_ERROR_LABEL.getIndexStaus();
            copyRestApi.updateCopyIndexStatus(copy.getUuid(), CopyIndexStatus.INDEX_FAIL.getIndexStaus(), errorCode);
            log.info("Update copy:{} index status: {} success.", copy.getUuid(),
                CopyIndexStatus.INDEX_FAIL.getIndexStaus());
        }
        return indexTask;
    }

    private boolean isSupportIndex(CopyBo copy) {
        String subType = copy.getResourceSubType();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(subType);
        ResourceProvider provider = providerManager.findProvider(ResourceProvider.class, protectedResource, null);
        boolean isSupportIndex = false;
        if (provider == null) {
            CopyIndexProvider copyIndexProvider = providerManager.findProvider(CopyIndexProvider.class, subType, null);
            if (copyIndexProvider != null) {
                isSupportIndex = copyIndexProvider.isSupportIndex();
            }
        } else {
            isSupportIndex = provider.isSupportIndex();
        }
        return isSupportIndex;
    }

    private boolean isIndex(CopyBo copy, String indexedMode) {
        if (CopyIndexConstants.GEN_INDEX_MANUAL.equals(indexedMode)) {
            return true;
        }
        PolicyBo policy = getThePolicyThCopyCreatedBy(copy);
        if (policy == null) {
            log.info("Do not find the policy.");
            return false;
        }
        // cloudbackup 索引标识从sla策略的高级参数中取，其他部署形态从保护对象的高级参数中取
        final JSONObject resourcePropertiesJsonObject = JSONObjectCovert.covertLowerUnderscoreKeyToLowerCamel(
                JSONObject.fromObject(copy.getResourceProperties()));
        final ProtectedObject protectedObject = resourcePropertiesJsonObject.toBean(ProtectedObject.class);
        Map<String, Object> extParameters = protectedObject.getExtParameters();
        if (extParameters != null) {
            if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(copy.getGeneratedBy()) && getAutoIndexFlag(
                protectedObject, CopyResourcePropertiesConstant.ARCHIVE_RES_AUTO_INDEX).isPresent()) {
                return getAutoIndexFlag(protectedObject, CopyResourcePropertiesConstant.ARCHIVE_RES_AUTO_INDEX).get();
            }
            if (CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(copy.getGeneratedBy()) && getAutoIndexFlag(
                protectedObject, CopyResourcePropertiesConstant.TAPE_ARCHIVE_AUTO_INDEX).isPresent()) {
                return getAutoIndexFlag(protectedObject, CopyResourcePropertiesConstant.TAPE_ARCHIVE_AUTO_INDEX).get();
            }
            if (CopyGeneratedByEnum.BY_BACKUP.value().equals(copy.getGeneratedBy()) && getAutoIndexFlag(protectedObject,
                CopyResourcePropertiesConstant.BACKUP_RES_AUTO_INDEX).isPresent()) {
                return getAutoIndexFlag(protectedObject, CopyResourcePropertiesConstant.BACKUP_RES_AUTO_INDEX).get();
            }
        }
        if (!deployTypeService.isCloudBackup() && !isManualArchive(JSONObject.fromObject(copy.getProperties()))) {
            return false;
        }
        JsonNode autoIndex = policy.getExtParameters().get("auto_index");
        if (autoIndex == null) {
            log.info("The auto index is close. copy id is {}", copy.getUuid());
            return false;
        }
        return autoIndex.asBoolean();
    }

    private boolean isManualArchive(JSONObject propertiesJson) {
        return propertiesJson.getBoolean(CopyPropertiesKeyConstant.IS_MANUAL_ARCHIVE, false);
    }

    private Optional<Boolean> getAutoIndexFlag(ProtectedObject protectedObject, String key) {
        Map<String, Object> extParameters = protectedObject.getExtParameters();
        Boolean resAutoIndex = MapUtils.getBoolean(extParameters, key, null);
        log.info("Resource: {} auto index tag is: {}", protectedObject.getResourceId(), resAutoIndex);
        return Optional.ofNullable(resAutoIndex);
    }

    private PolicyBo getThePolicyThCopyCreatedBy(CopyBo copy) {
        JSONObject propertiesJson = JSONObject.fromObject(copy.getProperties());
        if (isManualArchive(propertiesJson)) {
            return propertiesJson.getBean(CopyPropertiesKeyConstant.MANUAL_ARCHIVE_POLICY, PolicyBo.class);
        }
        String slaJsonStr = copy.getSlaProperties();
        SlaBo slaBo = JSONObject.toBean(slaJsonStr, SlaBo.class);
        return Optional.ofNullable(slaBo)
            .map(SlaBo::getPolicyList)
            .flatMap(
                policyList -> policyList.stream().filter(policy -> isPolicyThCopyCreatedBy(copy, policy)).findFirst())
            .orElse(null);
    }

    private boolean isPolicyThCopyCreatedBy(CopyBo copy, PolicyBo policy) {
        // 备份副本策略
        if (CopyGeneratedByEnum.BY_BACKUP.value().equals(copy.getGeneratedBy())) {
            return SlaPolicyTypeEnum.BACKUP.getName().equals(policy.getType());
        }

        // 归档副本策略
        if (copy.getIsArchived() && SlaPolicyTypeEnum.ARCHIVING.getName().equals(policy.getType())) {
            return true;
        }
        return false;
    }

    private CopyIndexTask buildIndexTask(CopyBo copy, String requestId, String indexedMode) {
        CopyIndexTask indexTask = new CopyIndexTask();
        indexTask.setRequestId(requestId);
        indexTask.setTriggerMode(indexedMode.toUpperCase(Locale.ENGLISH));

        CreateIndexCopyInfo copyInfo = new CreateIndexCopyInfo();
        BeanUtils.copyProperties(copy, copyInfo);
        copyInfo.setIndexed(CopyIndexStatus.INDEXED.getIndexStaus().equals(copy.getIndexed()));
        JSONObject properties = JSONObject.fromObject(copy.getProperties());
        copyInfo.setAggregation(properties.getBoolean("isAggregation", false));
        // 设置副本资源平台信息
        ResourceEntity tmpRes = JSONObject.fromObject(copy.getResourceProperties()).toBean(ResourceEntity.class);
        copyInfo.setResourcePlatform(tmpRes.getEnvironmentOsType());
        copyInfo.setEsn(copy.getDeviceEsn());
        // 手动备份副本会有userId，影响索引，置为null
        copyInfo.setUserId(null);
        indexTask.setCopyInfo(copyInfo);

        indexTask.setAgents(queryAgents(copy.getResourceType(), copy.getResourceSubType(), buildParameters(copy)));
        Copy newCopy = new Copy();
        BeanUtils.copyProperties(copy, newCopy);
        // 插件反馈下发请求时不需要protectEnv、protectObject参数，为适配复制副本索引功能，现去掉
        indexTask.setProtectObject(new TaskResource());
        indexTask.setProtectEnv(new TaskEnvironment());

        commonAgentService.supplyAgentCommonInfo(indexTask.getAgents());
        indexTask.setStorageRepository(
            taskRepositoryManager.getCopyRepoWithAuth(copy.getProperties(), copy.getStorageUnitId(),
                RepositoryTypeEnum.DATA.getType()).orElse(null));
        return indexTask;
    }

    private Map<String, String> buildParameters(CopyBo copy) {
        ResourceEntity resources = JSONObject.fromObject(copy.getResourceProperties()).toBean(ResourceEntity.class);
        Map<String, String> parameters = new HashMap<>();
        parameters.put(AgentKeyConstant.ENVIRONMENT_UUID_KEY, resources.getEnvironmentUuid());
        // 索引场景，将副本里面保存的agent信息下发给插件，插件可以优先使用绑定SLA时指定的agent执行索引任务
        String copyAgent = Optional.ofNullable(resources.getExtParameters())
            .map(params -> params.get(AgentKeyConstant.AGENTS_KEY))
            .filter(agentKey -> agentKey instanceof String)
            .map(agentKey -> (String) agentKey)
            .orElse(StringUtils.EMPTY);
        parameters.put(AgentKeyConstant.COPY_AGENT, copyAgent);
        if (!VerifyUtil.isEmpty(copy.getUserId())) {
            UserInnerResponse userInnerResponse = userService.getUserInfoByUserId(copy.getUserId());
            parameters.put(AgentKeyConstant.USER_INFO, JSONObject.writeValueAsString(userInnerResponse));
        }
        return parameters;
    }

    private List<Endpoint> queryAgents(String resourceType, String subType, Map<String, String> parameters) {
        ProtectAgentSelector selector = providerManager.findProvider(ProtectAgentSelector.class, resourceType, null);
        if (selector == null) {
            selector = defaultSelector;
        }
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(subType);
        return selector.select(protectedResource, parameters);
    }

    @Override
    public void deleteResourceIndexTask(String resourceId, String userId) {
        checkAutoIndex(resourceId);
        checkIndexStatus(resourceId);
        String requestId = UUID.randomUUID().toString();
        if (isDeleteAllCopyIndex(userId)) {
            deeUnifiedRestApi.deleteIndexTask(requestId, resourceId, StringUtils.EMPTY, StringUtils.EMPTY,
                StringUtils.EMPTY);
            log.info("Send delete resource copies index task to data enable engine successful!");
            copyRestApi.updateResourceCopyIndexStatus(resourceId, CopyIndexStatus.INDEX_DELETING.getIndexStaus(), "");
            return;
        }
        DomainInfoBo domainInfoBo = domainService.getDomainInfoByUserId(userId);
        List<String> copyIdList = copyRestApi.queryUserIndexCopyByResourceId(domainInfoBo.getUuid(), resourceId,
            CopyIndexStatus.INDEXED.getIndexStaus());
        if (CollectionUtils.isEmpty(copyIdList)) {
            log.info("Current user userId: {} has no indexed copy, no need to delete.", userId);
            return;
        }
        deeUnifiedRestApi.deleteIndexTask(buildDeleteCopyIndexRequest(requestId, resourceId, copyIdList));
        copyRestApi.updateCopyListIndexStatus(buildUpdateCopyIndexRequest(copyIdList,
            CopyIndexStatus.INDEX_DELETING.getIndexStaus(), ""));
    }

    private boolean isDeleteAllCopyIndex(String userId) {
        if (StringUtils.isEmpty(userId)) {
            return true;
        }
        DomainInfoBo domainInfoBo = domainService.getDomainInfoByUserId(userId);
        return DomainTypeEnum.SYS_PUBLIC_DOMAIN.getType() == domainInfoBo.getDomainType()
            || deployTypeService.isNotSupportRBACType();
    }

    private DeleteCopyIndexRequest buildDeleteCopyIndexRequest(String requestId, String resourceId,
        List<String> copyIdList) {
        DeleteCopyIndexRequest request = new DeleteCopyIndexRequest();
        request.setRequestId(requestId);
        request.setResourceId(resourceId);
        request.setCopyIdList(copyIdList);
        request.setChainId(StringUtils.EMPTY);
        request.setUserId(StringUtils.EMPTY);
        return request;
    }

    private UpdateCopyIndexStatusRequest buildUpdateCopyIndexRequest(List<String> copyIdList, String indexStatus,
        String errorCode) {
        UpdateCopyIndexStatusRequest request = new UpdateCopyIndexStatusRequest();
        request.setCopyIdList(copyIdList);
        request.setIndexStatus(indexStatus);
        request.setErrorCode(errorCode);
        return request;
    }

    private void checkAutoIndex(String resourceId) {
        // 资源若绑定的sla的自动索引打开，就不能清理资源索引
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(resourceId);
        resOptional.flatMap(protectedResource -> Optional.ofNullable(protectedResource.getProtectedObject())
            .map(protectedObject -> slaRestApi.querySlaById(protectedObject.getSlaId()))
            .flatMap(slaBo -> slaBo.getPolicyList()
                .stream()
                .filter(policy -> policy.getExtParameters().get(CopyIndexConstants.AUTO_INDEX) != null
                    && policy.getExtParameters().get(CopyIndexConstants.AUTO_INDEX).asBoolean(false))
                .findFirst())).ifPresent(autoIndexPolicy -> {
            log.error("The sla auto index of resource protection is enabled.resource id {}", resourceId);
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_SLA_AUTO_INDEX_ENABLED,
                new String[] {resOptional.get().getProtectedObject().getSlaName()}, "The sla auto index is enabled");
        });
    }

    private void checkIndexStatus(String resourceId) {
        // 若有副本在索引中，不能清除资源索引
        List<Copy> copys = copyRestApi.queryCopiesByResourceIdAndIndexStatus(resourceId,
            CopyIndexStatus.INDEXING.getIndexStaus());
        if (!copys.isEmpty()) {
            log.error("It doesn't support deleting resource index when copy is indexing.resource id {}", resourceId);
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_INDEXING, "Have copy indexing.");
        }
    }

    /**
     * 删除副本索引任务
     *
     * @param requestId 请求id
     * @param copyId 副本id
     */
    @Override
    public void deleteCopyIndex(String requestId, String copyId) {
        // 删除已经索引的副本，删除时需删除索引
        Copy copy = copyRestApi.queryCopyByID(copyId);
        if (VerifyUtil.isEmpty(copy)) {
            log.error("Copy is not exists. copy id {}", copyId);
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Copy is not exists.");
        }
        if (!CopyIndexStatus.INDEXED.getIndexStaus().equals(copy.getIndexed())) {
            log.debug("Copy need not delete index, requestId: {}", requestId);
            return;
        }
        String uuid = StringUtils.isEmpty(requestId) ? UUID.randomUUID().toString() : requestId;
        deeUnifiedRestApi.deleteIndexTask(uuid, copy.getResourceId(), copyId, copy.getChainId(), copy.getUserId());
        log.debug("Delete copy index task to data enable engine successful! requestId: {}", uuid);
    }

    @Override
    public void forwardCreateIndex(String esn, String copyId) {
        log.info("start forward request to remote node esn:{}", esn);
        ClusterRequestInfo clusterRequestInfo = memberClusterService.getClusterRequestInfo(esn);
        String ip = IpUtil.getAvailableIp(clusterRequestInfo.getIp());
        URI uri = ClusterUriUtil.buildURI(ip, clusterRequestInfo.getPort());
        try {
            targetClusterRestApi.createIndex(uri, clusterRequestInfo.getToken(), copyId);
        } catch (FeignException | LegoCheckedException | LegoUncheckedException e) {
            log.error("request to remote node(esn:{}) failed", esn, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT,
                "request to remote node failed esn:" + esn);
        }
    }
}
