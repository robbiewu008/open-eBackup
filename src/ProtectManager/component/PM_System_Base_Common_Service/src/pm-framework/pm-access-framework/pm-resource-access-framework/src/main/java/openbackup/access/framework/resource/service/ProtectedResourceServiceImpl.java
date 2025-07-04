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

import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.system.base.label.service.LabelService;
import com.huawei.oceanprotect.system.base.user.bo.DomainInfoBo;
import com.huawei.oceanprotect.system.base.user.bo.ResourceSetBo;
import com.huawei.oceanprotect.system.base.user.common.constant.UserConstants;
import com.huawei.oceanprotect.system.base.user.common.helper.ResourceSetRelationHelper;
import com.huawei.oceanprotect.system.base.user.controller.response.ResourceSetRoleResponse;
import com.huawei.oceanprotect.system.base.user.entity.DomainResourceObjectEntity;
import com.huawei.oceanprotect.system.base.user.entity.ResourceSetResourceBo;
import com.huawei.oceanprotect.system.base.user.entity.ResourceSetResourceEntity;
import com.huawei.oceanprotect.system.base.user.service.DomainResourceSetService;
import com.huawei.oceanprotect.system.base.user.service.DomainService;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetResourceService;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetService;
import com.huawei.oceanprotect.system.base.user.service.UserService;
import com.huawei.oceanprotect.system.sdk.dto.SystemSwitchDto;
import com.huawei.oceanprotect.system.sdk.enums.SwitchNameEnum;
import com.huawei.oceanprotect.system.sdk.enums.SwitchStatusEnum;
import com.huawei.oceanprotect.system.sdk.service.SystemSwitchInternalService;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.TypeReference;
import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.conditions.update.LambdaUpdateWrapper;
import com.baomidou.mybatisplus.core.conditions.update.UpdateWrapper;
import com.google.common.collect.ImmutableList;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.client.AntiRansomwareDeviceApi;
import openbackup.access.framework.resource.client.model.LunInfo;
import openbackup.access.framework.resource.client.model.ResourceInfo;
import openbackup.access.framework.resource.client.model.UpdateFileSystemRequest;
import openbackup.access.framework.resource.client.model.UpdateLunInfoReq;
import openbackup.access.framework.resource.dto.ResourceDependencyRelation;
import openbackup.access.framework.resource.model.CopyListParams;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceExtendInfoMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceMapper;
import openbackup.access.framework.resource.persistence.dao.ResourceGroupMapper;
import openbackup.access.framework.resource.persistence.dao.ResourceGroupMemberMapper;
import openbackup.access.framework.resource.persistence.model.ProtectedResourceExtendInfoPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.access.framework.resource.persistence.model.ResourceGroupMemberPo;
import openbackup.access.framework.resource.persistence.model.ResourceGroupPo;
import openbackup.access.framework.resource.persistence.model.ResourceRepositoryQueryParams;
import openbackup.access.framework.resource.service.proxy.ProxyFactory;
import openbackup.access.framework.resource.util.ResourceConstant;
import openbackup.access.framework.resource.util.ResourceFilterUtil;
import openbackup.access.framework.resource.util.ResourceUtil;
import openbackup.data.access.framework.core.common.exception.LockNotObtainedException;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupModifyReq;
import openbackup.data.protection.access.provider.sdk.backup.ResourceExtendInfoConstants;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.copy.CopyServiceSdk;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionRejectException;
import openbackup.data.protection.access.provider.sdk.plugin.PluginExtensionInvokeContext;
import openbackup.data.protection.access.provider.sdk.plugin.ResourceExtensionManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.NextBackupParams;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceGroupResult;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.VstoreResourceQueryParam;
import openbackup.data.protection.access.provider.sdk.resource.model.AgentTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.model.ExecuteScanRes;
import openbackup.data.protection.access.provider.sdk.resource.model.ResourceExtendInfoKeyConstants;
import openbackup.data.protection.access.provider.sdk.resource.model.ResourceScanDto;
import openbackup.data.protection.access.provider.sdk.resource.model.ResourceUpsertRes;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceFilterConditionParam;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceFilterParam;
import openbackup.system.base.common.aspect.OperationLogService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.CommonOperationCode;
import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.common.constants.LegoInternalEvent;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.enums.UserTypeEnum;
import openbackup.system.base.common.errors.ResourceLockErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.log.constants.EventTarget;
import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.SortingParamRequest;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.model.job.request.QueryJobRequest;
import openbackup.system.base.common.utils.CollectionUtils;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.pack.lock.Lock;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.quota.QuotaService;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.enums.ProtectionStatusEnum;
import openbackup.system.base.sdk.resource.model.ProtectionBatchOperationReq;
import openbackup.system.base.sdk.resource.model.ProtectionCreationDto;
import openbackup.system.base.sdk.resource.model.ProtectionModifyDto;
import openbackup.system.base.sdk.resource.model.ProtectionResourceDto;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.sdk.resource.model.UpdateCopyUserObjectReq;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.hostagent.AgentQueryService;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.DefaultRoleHelper;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.OpServiceUtil;

import org.apache.commons.collections.MapUtils;
import org.apache.commons.collections4.ListUtils;
import org.apache.commons.lang3.ArrayUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Queue;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * 受保护资源服务实现类
 *
 */
@Service
@Slf4j
public class ProtectedResourceServiceImpl implements ResourceService {
    private static final Integer MAX_DEPENDENCY_RESOURCE_NUM = 1000;
    private static final Integer MAX_OBTAIN_LOCK_TIME = 3;
    private static final Integer MAX_PAGE_NUM = 500;
    private static final String SIZE = "size";
    private static final Integer QUERY_PAGE_SIZE = 10000;
    private static final Integer PARTITION_SIZE = 10000;
    private static final Integer MAX_DORADO_RESOURCE_NUM = 100000;
    private static final Integer TIMER_INTERVAL = 1000 * 60 * 30;
    private static final Integer TIMES = 24;
    private static final String RESOURCE_CREATE_LOCK_KEY = "/resource/create/lock/";

    private static final String AUTO_SCAN_CONFIG_PATH = "functions.scan.auto-scan";

    private static final String UPDATE_DELETE_CONNECTION_PREFIX = "-";
    private static final String UPDATE_DELETE_RESOURCE_PREFIX = "#";

    private static final String SCAN_RESOURCE_EXCEED_LIMIT_LABEL = "job_log_scan_resource_exceed_limit_label";

    private static final String FILE_SYSTEM = "CloudBackupFileSystem";

    /**
     * 开启了lanFree的配置 的key的值
     */
    private static final String IS_LAN_FREE = "1";

    private static final String PAGE_NO = "pageNo";

    private static final String PAGE_SIZE = "pageSize";

    private static final String CPU_RATE = "cpuRate";

    private static final String MEM_RATE = "memRate";

    private static final String PRE_SCRIPT = "pre_script";

    private static final String POST_SCRIPT = "post_script";

    private static final String ENABLE_SECURITY_ARCHIVE = "enable_security_archive";

    private static final String WORM_SWITCH = "worm_switch";

    private static final String AGENTS = "agents";

    private static final String SNAP_DELETE_SPEED = "snap_delete_speed";

    private static final String IS_CONSISTENT = "is_consistent";

    private static final String BACKUP_RES_AUTO_INDEX = "backup_res_auto_index";

    private static final String ARCHIVE_RES_AUTO_INDEX = "archive_res_auto_index";

    private static final String APPLY_TO_ALL = "APPLY_TO_ALL";

    private static final String OVERWRITE = "overwrite";

    private static final String BINDING_POLICY = "binding_policy";

    private static final String RESOURCE_FILTERS = "resource_filters";

    private static final String RESOURCE_TAG_FILTERS = "resource_tag_filters";

    private static final String DISK_INFO = "disk_info";

    private static final List<String> HCS_OP_LIST = ImmutableList.of(ResourceSubTypeEnum.HCS_GAUSSDB_PROJECT.getType(),
            ResourceSubTypeEnum.HCS_CONTAINER.getType(), ResourceSubTypeEnum.HDFS.getType(),
            ResourceSubTypeEnum.HIVE.getType(), ResourceSubTypeEnum.HBASE.getType(),
            ResourceSubTypeEnum.GAUSSDB_DWS.getType());

    private static final List<JobStatusEnum> RUNNING_AND_PENDING_STATUS_LIST =
        ImmutableList.of(JobStatusEnum.PENDING, JobStatusEnum.RUNNING, JobStatusEnum.ABORTING, JobStatusEnum.READY);

    private static final List<String> UPDATE_PROTECT_TYPE_LIST = ImmutableList.of(
        ResourceSubTypeEnum.CNWARE.getType(), ResourceSubTypeEnum.NUTANIX.getType(),
        ResourceTypeEnum.VIRTUALIZATION.getType(), ResourceSubTypeEnum.APSARA_STACK.getType(),
        ResourceSubTypeEnum.OPEN_STACK.getType(), ResourceTypeEnum.HCS.getType());

    private static final List<String> NEED_UPDATE_PROTECT_STATUS_TYPE_LIST = ImmutableList.of(
        ResourceSubTypeEnum.CNWARE_VM.getType(), ResourceSubTypeEnum.NUTANIX_VM.getType(),
        ResourceSubTypeEnum.HYPER_V_VM.getType(), ResourceSubTypeEnum.APS_INSTANCE.getType(),
        ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType(), ResourceSubTypeEnum.HCS_CLOUD_HOST.getType());

    private static final List<String> FC_AND_FC_ONE_TYPE_LIST = ImmutableList.of(
        ResourceSubTypeEnum.FUSION_COMPUTE.getType(), ResourceSubTypeEnum.FUSION_ONE_COMPUTE.getType());

    /**
     * 资源的扩展字段中有这个key,若为false则跳过新增或更新操作.用数据库中已有的资源
     */
    private static final String SCAN_SKIP_UPDATE_AND_INSERT = "scanSkipUpdateAndInsert";

    private static final String UPDATE_COPY_RESOURCE_NAME = "resource_name";

    private static final String TASK_BACKUP = "Backup";

    private static final String TASK_ARCHIVE = "CloudArchive";

    private static final String TASK_REPLICATED = "Replicated";

    private static final String INCREASE = "increase";

    private static final String REDUCE = "reduce";

    private static final String DATA_SIZE_DEFAULT_VALUE = "0";

    @Value("${MAX_RESOURCE_NUM:20000}")
    private int maxResourceNum;
    private ProtectedResourceExtendInfoMapper resourceExtendInfoMapper;
    private ProviderManager providerManager;
    private LockService lockService;
    private ResourceExtensionManager resourceExtensionManager;
    private JobScheduleService jobScheduleService;
    private DeployTypeService deployTypeService;
    private JobService jobService;
    private SystemSwitchInternalService systemSwitchInternalService;
    private ResourceScanService resourceScanService;
    private ResourceAlarmService resourceAlarmService;
    private SessionService sessionService;
    private AgentQueryService agentQueryService;
    private ResourceGroupMemberMapper resourceGroupMemberMapper;
    private ResourceGroupMapper resourceGroupMapper;
    private DomainService domainService;
    private DomainResourceSetService domainResourceSetService;
    private ResourceSetApi resourceSetApi;

    private AntiRansomwareDeviceApi antiRansomwareDeviceApi;

    @Autowired
    private LabelService labelService;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private CopyServiceSdk copyServiceSdk;

    @Autowired
    private ProtectedResourceRepository repository;

    @Autowired
    private ProtectedResourceMonitorService protectedResourceMonitorService;

    @Autowired
    private ProtectedResourceMapper protectedResourceMapper;

    @Autowired
    private ProtectedResourceDecryptService decryptService;

    @Autowired
    private MessageTemplate<String> messageTemplate;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private UserService userService;

    @Autowired
    private QuotaService quotaService;

    @Autowired
    private CopyManagerService copyManagerService;

    @Autowired
    private ResourceSetService resourceSetService;

    @Autowired
    private ResourceSetResourceService resourceSetResourceService;

    @Autowired
    private ProtectObjectRestApi protectObjectRestApi;

    @Autowired
    private OperationLogService operationLogService;

    @Autowired
    public void setJobScheduleService(JobScheduleService jobScheduleService) {
        this.jobScheduleService = jobScheduleService;
    }

    @Autowired
    public void setResourceGroupMemberMapper(ResourceGroupMemberMapper resourceGroupMemberMapper) {
        this.resourceGroupMemberMapper = resourceGroupMemberMapper;
    }

    @Autowired
    public void setResourceGroupMapper(ResourceGroupMapper resourceGroupMapper) {
        this.resourceGroupMapper = resourceGroupMapper;
    }

    @Autowired
    public void setDeployTypeService(DeployTypeService deployTypeService) {
        this.deployTypeService = deployTypeService;
    }

    @Autowired
    public void setResourceScanService(ResourceScanService resourceScanService) {
        this.resourceScanService = resourceScanService;
    }

    @Autowired
    public void setLockService(LockService lockService) {
        this.lockService = lockService;
    }

    @Autowired
    public void setResourceExtendInfoMapper(final ProtectedResourceExtendInfoMapper resourceExtendInfoMapper) {
        this.resourceExtendInfoMapper = resourceExtendInfoMapper;
    }

    @Autowired
    public void setProviderManager(final ProviderManager providerManager) {
        this.providerManager = providerManager;
    }

    @Autowired
    public void setResourceExtensionManager(final ResourceExtensionManager resourceExtensionManager) {
        this.resourceExtensionManager = resourceExtensionManager;
    }

    @Autowired
    public void setSystemSwitchInternalService(SystemSwitchInternalService systemSwitchInternalService) {
        this.systemSwitchInternalService = systemSwitchInternalService;
    }

    @Autowired
    public void setJobService(JobService jobService) {
        this.jobService = jobService;
    }

    @Autowired
    public void setResourceAlarmService(ResourceAlarmService resourceAlarmService) {
        this.resourceAlarmService = resourceAlarmService;
    }

    @Autowired
    public void setSessionService(SessionService sessionService) {
        this.sessionService = sessionService;
    }

    @Autowired
    public void setAgentQueryService(AgentQueryService agentQueryService) {
        this.agentQueryService = agentQueryService;
    }

    @Autowired
    public void setAntiRansomwareDeviceApi(AntiRansomwareDeviceApi antiRansomwareDeviceApi) {
        this.antiRansomwareDeviceApi = antiRansomwareDeviceApi;
    }

    @Autowired
    public void setResourceSetApi(ResourceSetApi resourceSetApi) {
        this.resourceSetApi = resourceSetApi;
    }

    @Autowired
    public void setDomainService(DomainService domainService) {
        this.domainService = domainService;
    }

    @Autowired
    public void setDomainResourceSetService(DomainResourceSetService domainResourceSetService) {
        this.domainResourceSetService = domainResourceSetService;
    }

    @Override
    @Transactional
    public String[] create(ProtectedResource[] resources, boolean shouldExecuteAutoScan) {
        if (resources == null) {
            return new String[0];
        }
        int length = resources.length;
        String[] uuidList = new String[length];
        checkUpperLimit(length);
        for (int index = 0; index < length; index++) {
            ProtectedResource resource = resources[index];
            ResourceProvider provider = providerManager.findProvider(ResourceProvider.class, resource, null);
            if (provider != null) {
                provider.check(resource);
            }
            String uuid = createResourceWithDependency(resource, shouldExecuteAutoScan);
            uuidList[index] = uuid;
        }
        return uuidList;
    }

    @Override
    public ActionResult[] check(ProtectedResource resource, boolean isCheckResult) {
        log.info("[ProtectedResource] check resource uuid: {}", resource.getUuid());
        Object result = protectedResourceMonitorService.invoke("check", resource, event -> {
            ResourceConnectionCheckProvider provider = providerManager
                    .findProvider(ResourceConnectionCheckProvider.class, event.getResource());
            if (isCheckResult) {
                return provider.checkConnection(event.getResource());
            } else {
                return provider.tryCheckConnection(event.getResource());
            }
        });
        if (result instanceof ResourceCheckContext) {
            return ((ResourceCheckContext) result).getActionResults().toArray(new ActionResult[0]);
        }
        return new ActionResult[0];
    }

    private String createResource(ProtectedResource resource) {
        String uuid;
        // 防呆逻辑，避免外部传入不需要使用的environment对象，
        // 导致多个资源批量创建场景，environment对象的敏感字段属性被加密多次
        ProtectedEnvironment environment = resource.getEnvironment();
        resource.setEnvironment(null);
        try {
            Object value;
            ResourceProvider resourceProvider = providerManager.findProvider(ResourceProvider.class, resource, null);
            if (resourceProvider != null && ResourceConstants.SOURCE_TYPE_AUTO_SCAN.equals(resource.getSourceType())
                    && resourceProvider.getResourceFeature().isShouldSaveDirectlyWhenScan()) {
                ProtectedResourceEvent event = new ProtectedResourceEvent();
                event.setResource(resource);
                value = createResource(event);
            } else {
                value = protectedResourceMonitorService.invoke("create", resource, this::createResource);
            }
            if (value instanceof String) {
                uuid = (String) value;
            } else {
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR);
            }
        } finally {
            resource.setEnvironment(environment);
        }
        messageTemplate.send("resource.added", new JSONObject().set("resource_id", uuid));
        return uuid;
    }

    private void checkUpperLimit(int size) {
        int maxSize = maxResourceNum;
        if (deployTypeService.isHyperDetectDeployType()) {
            maxSize = MAX_DORADO_RESOURCE_NUM;
        }
        if (maxSize == -1) {
            return;
        }
        LambdaQueryWrapper<ProtectedResourcePo> wrapper = new LambdaQueryWrapper<>();
        wrapper.ne(ProtectedResourcePo::getType, ResourceTypeEnum.PLUGIN.getType());
        int currentSize = protectedResourceMapper.selectCount(wrapper).intValue();
        int afterCreatDbSize = currentSize + size;
        log.info("Check resource size. to add size:{}, currentSize: {}, limit: {}", size, currentSize, maxSize);
        if (afterCreatDbSize > maxSize) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_NUM_EXCEED_LIMIT,
                    new String[]{String.valueOf(maxSize)},
                    "Do not add resources, the number of resources exceeds the upper limit.");
        }
    }

    private String createResourceWithDependency(ProtectedResource resource, boolean shouldExecuteAutoScan) {
        String resourceUuid = VerifyUtil.isEmpty(resource.getUuid()) ? UUIDGenerator.getUUID() : resource.getUuid();
        resource.setUuid(resourceUuid);
        log.info("create resource dependency, resource uuid: {}", resource.getUuid());
        // set root uuid
        if (resource.getRootUuid() == null && resource.getParentUuid() != null) {
            String rootUuid = protectedResourceMapper.queryRootUuid(resource.getParentUuid());
            if (rootUuid == null) {
                throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                        "not found root resource for " + resource.getParentUuid());
            }
            resource.setRootUuid(rootUuid);
        }
        Map<String, ProtectedResource> existResource = findExistResource(resource);
        checkHostTrustFromExistResource(resource, existResource);
        checkAndFillAuthorizion(resource, existResource);
        handleResourceDependency(false, resource, existResource);
        createResource(resource);
        Map<String, List<ProtectedResource>> dependencies = resource.getDependencies();
        if (dependencies != null && !Objects.equals(resourceUuid, resource.getUuid())) {
            throw new LegoCheckedException("create resource uuid has changed. resource name is: " + resource.getName());
        }
        if (shouldExecuteAutoScan) {
            resourceExtensionManager.invoke(resource.getSubType(), AUTO_SCAN_CONFIG_PATH,
                    new PluginExtensionInvokeContext<ProtectedResource, Void>() {
                        @Override
                        public ProtectedResource getParams() {
                            return resource;
                        }
                    });
        }
        return resourceUuid;
    }

    private void checkAndFillAuthorizion(ProtectedResource resource, Map<String, ProtectedResource> existResource) {
        // 过滤共享agent
        List<String> sharedAgentIds = agentQueryService.querySharedAgentIds();
        List<String> userIds = existResource.values()
            .stream()
            .filter((res) -> !sharedAgentIds.contains(res.getRootUuid()))
            .map(ProtectedResource::getUserId)
            .distinct()
            .collect(Collectors.toList());
        if (userIds.size() > 1) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_AUTHORIZE_INCONSISTENT,
                "resource has more than one authorized user");
        }
        String userId;
        String authorizedUser;
        if (userIds.size() == 0 || VerifyUtil.isNone(userIds.get(0))) {
            TokenBo.UserBo currentUser = sessionService.getCurrentUser();
            if (currentUser == null || DefaultRoleHelper.isAdmin(currentUser.getId())) {
                return;
            }
            userId = currentUser.getId();
            authorizedUser = currentUser.getName();
        } else {
            ProtectedResource protectedResource = existResource.values()
                    .stream()
                    .filter(r -> r.getUserId() != null)
                    .findFirst()
                    .get();
            userId = protectedResource.getUserId();
            authorizedUser = protectedResource.getAuthorizedUser();
        }
        Queue<ProtectedResource> queue = new LinkedList<>();
        queue.offer(resource);
        while (queue.size() > 0) {
            ProtectedResource poll = queue.poll();
            if (poll == null) {
                continue;
            }
            // 共享agent及其子资源不授权用户
            if (Objects.isNull(poll.getUserId()) && !sharedAgentIds.contains(poll.getRootUuid())) {
                poll.setUserId(userId);
                poll.setAuthorizedUser(authorizedUser);
            }
            addDependencyToQueue(poll.getDependencies(), queue, true);
        }
    }

    private void tryLockResource(Set<String> uuids, Set<Lock> locks) {
        if (org.springframework.util.CollectionUtils.isEmpty(uuids)) {
            return;
        }
        log.info("try lock resources: {}", JsonUtil.json(uuids));
        for (String uuid : uuids) {
            Lock lock = lockService.createSQLDistributeLock(RESOURCE_CREATE_LOCK_KEY + uuid);
            locks.add(lock);
            if (!lock.tryLock(MAX_OBTAIN_LOCK_TIME, TimeUnit.SECONDS)) {
                throw new LegoCheckedException("resource can not obtain lock. resource id: " + uuid);
            } else {
                log.debug("try lock success. id: " + uuid);
            }
        }
    }

    private void unlockResource(Set<Lock> locks) {
        if (locks == null) {
            return;
        }
        for (Lock lock : locks) {
            try {
                lock.unlock();
            } catch (LegoCheckedException e) {
                log.error("resource lock unlock failed.");
            }
        }
    }

    private void handleResourceDependency(boolean isOverwrite, ProtectedResource protectedResource,
                                          Map<String, ProtectedResource> existResource) {
        if (protectedResource.getDependencies() == null) {
            return;
        }
        List<ResourceNode> resourceNodes = new ArrayList<>();
        collectResourceDependencyByBfs(protectedResource, resourceNodes, existResource);
        Set<Lock> locks = new HashSet<>();
        try {
            tryLockResource(existResource.keySet(), locks);
            for (int i = resourceNodes.size() - 1; i >= 0; i--) {
                ResourceNode resourceNode = resourceNodes.get(i);
                if (Objects.equals(ResourceNode.add, resourceNode.type)) {
                    createResource(resourceNode.resource);
                } else {
                    update(isOverwrite, resourceNode.resource);
                }
            }
        } finally {
            unlockResource(locks);
        }
    }

    private void collectResourceDependencyByBfs(ProtectedResource protectedResource, List<ResourceNode> resourceNodes,
                                                Map<String, ProtectedResource> existResource) {
        Map<String, List<ProtectedResource>> dependencies = protectedResource.getDependencies();
        if (dependencies == null) {
            return;
        }
        for (Map.Entry<String, List<ProtectedResource>> entry : dependencies.entrySet()) {
            String dependencyKey = entry.getKey();
            List<ProtectedResource> protectedResources = entry.getValue();
            if (protectedResources == null) {
                continue;
            }
            for (ProtectedResource resource : protectedResources) {
                if (resource.getUuid() != null && existResource.containsKey(resource.getUuid())) {
                    // update
                    resourceNodes.add(new ResourceNode(ResourceNode.update,
                            collectUpdateDependency(resource, protectedResource, existResource, dependencyKey)));
                } else {
                    // add
                    resourceNodes.add(new ResourceNode(ResourceNode.add,
                            collectAddDependency(resource, protectedResource, dependencyKey)));
                }
                collectResourceDependencyByBfs(resource, resourceNodes, existResource);
            }
        }
    }

    private ProtectedResource collectAddDependency(ProtectedResource resource, ProtectedResource upperResource,
            String dependencyKey) {
        resource.setUuid(UUIDGenerator.getUUID());
        if (Objects.equals(dependencyKey, ResourceConstants.CHILDREN)) {
            resource.setParentUuid(upperResource.getUuid());
            resource.setRootUuid(upperResource.getRootUuid());
        }
        resource.setExtendInfoByKey(getCitationKey(dependencyKey), upperResource.getUuid());
        return resource;
    }

    private ProtectedResource collectUpdateDependency(ProtectedResource resource, ProtectedResource upperResource,
            Map<String, ProtectedResource> existResource, String dependencyKey) {
        ProtectedResource update = resource;
        ProtectedResource originResource = null;
        if (existResource.containsKey(resource.getUuid())) {
            originResource = existResource.get(resource.getUuid());
            Map<String, String> mergeExtendInfo = mergeExtendInfo(resource.getExtendInfo(),
                    originResource.getExtendInfo());
            update = originResource;
            BeanUtils.copyProperties(resource, update);
            update.setExtendInfo(mergeExtendInfo);
        }
        if (shouldCreateCitation(upperResource, update,
                Optional.ofNullable(originResource).map(ProtectedResource::getExtendInfo).orElse(null), dependencyKey,
                existResource)) {
            update.setExtendInfoByKey(getCitationKey(dependencyKey), upperResource.getUuid());
        }
        return update;
    }

    private Map<String, String> mergeExtendInfo(Map<String, String> updateExtendInfo,
            Map<String, String> exsitExtendInfo) {
        Map<String, String> res = new HashMap<>();
        if (exsitExtendInfo != null) {
            res.putAll(exsitExtendInfo);
        }
        if (updateExtendInfo != null) {
            res.putAll(updateExtendInfo);
        }
        return res;
    }

    private boolean shouldCreateCitation(ProtectedResource protectedResource, ProtectedResource dependencyResource,
            Map<String, String> originExtendInfo, String dependencyKey, Map<String, ProtectedResource> existResource) {
        if (dependencyResource == null) {
            return true;
        }
        if (existResource.containsKey(protectedResource.getUuid())
                && existResource.containsKey(dependencyResource.getUuid())) {
            if (originExtendInfo == null) {
                return true;
            }
            String citationKeyPrefix = getCitationKeyPrefix(dependencyKey);
            for (Map.Entry<String, String> entry : originExtendInfo.entrySet()) {
                if (entry.getKey().startsWith(citationKeyPrefix)
                        && Objects.equals(entry.getValue(), protectedResource.getUuid())) {
                    return false;
                }
            }
        }
        return true;
    }

    private String getCitationKey(String lastDependencyKey) {
        return ResourceConstants.CITATION + ResourceConstants.CITATION_SEPERATOR + lastDependencyKey
                + ResourceConstants.CITATION_SEPERATOR + UUIDGenerator.getUUID();
    }

    private String getCitationKeyPrefix(String lastDependencyKey) {
        return ResourceConstants.CITATION + ResourceConstants.CITATION_SEPERATOR + lastDependencyKey;
    }

    private void checkHostTrustFromExistResource(ProtectedResource root, Map<String, ProtectedResource> resourceMap) {
        List<ProtectedResource> protectedResources = new ArrayList<>();
        findNotDeleteExistResourceFromDependency(root, resourceMap, protectedResources);
        List<ProtectedEnvironment> protectedEnvironments = protectedResources.stream()
                .filter(e -> e instanceof ProtectedEnvironment).map(e -> (ProtectedEnvironment) e)
                .collect(Collectors.toList());
        checkHostIfBeTrusted(protectedEnvironments);
    }

    private void findNotDeleteExistResourceFromDependency(ProtectedResource root,
            Map<String, ProtectedResource> resourceMap, List<ProtectedResource> protectedResources) {
        Map<String, List<ProtectedResource>> dependencies = root.getDependencies();
        if (dependencies == null) {
            return;
        }
        if (resourceMap == null) {
            return;
        }
        for (String dependencyKey : dependencies.keySet()) {
            boolean shouldDelete = shouldDeletedByDependencyKey(dependencyKey);
            if (shouldDelete) {
                continue;
            }
            for (ProtectedResource protectedResource : dependencies.get(dependencyKey)) {
                if (protectedResource.getUuid() == null || resourceMap.get(protectedResource.getUuid()) == null) {
                    continue;
                }
                protectedResources.add(resourceMap.get(protectedResource.getUuid()));
                findNotDeleteExistResourceFromDependency(protectedResource, resourceMap, protectedResources);
            }
        }
    }

    private Map<String, ProtectedResource> findExistResource(ProtectedResource protectedResource) {
        return findExistResource(protectedResource, false);
    }

    private Map<String, ProtectedResource> findExistResource(ProtectedResource protectedResource,
            boolean shouldExcludeDelete) {
        int count = 0;
        Queue<ProtectedResource> queue = new LinkedList<>();
        Set<String> exsitUUids = new HashSet<>();
        Optional.ofNullable(protectedResource.getUuid()).ifPresent(exsitUUids::add);
        Map<String, List<ProtectedResource>> dependencies = Optional.ofNullable(protectedResource.getDependencies())
                .orElse(new HashMap<>());
        addDependencyToQueue(dependencies, queue, shouldExcludeDelete);
        while (queue.size() > 0) {
            ProtectedResource poll = queue.poll();
            if (poll == null) {
                continue;
            }
            count++;
            Optional.ofNullable(poll.getUuid()).ifPresent(exsitUUids::add);
            Map<String, List<ProtectedResource>> pollDependencies = poll.getDependencies();
            addDependencyToQueue(pollDependencies, queue, shouldExcludeDelete);
        }
        if (count > MAX_DEPENDENCY_RESOURCE_NUM) {
            throw new LegoCheckedException("resource number exceeds the maximum limit");
        }
        if (exsitUUids.size() == 0) {
            return new HashMap<>();
        }
        ResourceQueryParams params = new ResourceQueryParams();
        params.setShouldDecrypt(true);
        params.setPage(0);
        params.setSize(MAX_DEPENDENCY_RESOURCE_NUM);
        params.setConditions(Collections.singletonMap("uuid", exsitUUids));
        params.setShouldIgnoreOwner(true);
        PageListResponse<ProtectedResource> response = query(params);
        return response.getRecords().stream()
                .collect(Collectors.toMap(ProtectedResource::getUuid, Function.identity()));
    }

    private void addDependencyToQueue(Map<String, List<ProtectedResource>> dependencies, Queue<ProtectedResource> queue,
            boolean shouldExcludeDelete) {
        if (VerifyUtil.isEmpty(dependencies)) {
            return;
        }
        dependencies.forEach((key, resourceList) -> {
            if (!shouldDeletedByDependencyKey(key) || !shouldExcludeDelete) {
                queue.addAll(resourceList);
            }
        });
    }

    private String createResource(ProtectedResourceEvent event) {
        ProtectedResource resource = event.getResource();
        Authentication authentication = resource.getAuth();
        if (authentication != null) {
            resource.setAuth(mergeAuthentication(new Authentication[]{authentication}));
        }
        if (resource.getProtectionStatus() != null) {
            // 初始默认设置为未保护
            resource.setProtectionStatus(0);
        }
        initRootUuid(resource);
        // 如果父资源有授权，子资源也授权
        if (VerifyUtil.isEmpty(resource.getUserId())) {
            setResourceUserThroughParent(resource);
        }
        createResourceSetRelation(resource);
        return repository.create(resource);
    }

    private void setResourceUserThroughParent(ProtectedResource resource) {
        if (resource.getParentUuid() == null) {
            log.warn("parent uuid is null, res id:{} ", resource.getUuid());
            return;
        }
        ProtectedResourcePo protectedResourcePo = protectedResourceMapper.selectById(resource.getParentUuid());
        if (protectedResourcePo == null) {
            log.warn("can not find parent resource. res id: {}, parent id: {}", resource.getUuid(),
                resource.getParentUuid());
            return;
        }
        resource.setUserId(protectedResourcePo.getUserId());
        resource.setAuthorizedUser(protectedResourcePo.getAuthorizedUser());
    }

    /**
     * 创建资源集填充
     *
     * @param resource 资源
     */
    private void createResourceSetRelation(ProtectedResource resource) {
        ResourceSetResourceBo resourceSetResourceBo = new ResourceSetResourceBo();
        if (!ResourceSubTypeEnum.FILESET.getType().equals(resource.getSubType())) {
            resourceSetResourceBo.setParentResourceObjectId(resource.getParentUuid());
        }
        resourceSetResourceBo.setResourceObjectId(resource.getUuid());
        resourceSetResourceBo.setUserId(resource.getUserId());
        resourceSetResourceBo.setIsManualAdd(Boolean.TRUE);
        if (ResourceSetTypeEnum.AGENT.getType()
            .equals(ResourceSubTypeEnum.getScopeModuleBySubType(resource.getSubType()))) {
            resourceSetResourceBo.setType(ResourceSetTypeEnum.AGENT);
            if (VerifyUtil.isEmpty(resource.getUserId())) {
                resourceSetResourceBo.setDomainIdList(resourceSetApi.getPublicDomainIdList());
            }
        } else {
            resourceSetResourceBo.setType(ResourceSetTypeEnum.RESOURCE);
        }
        resourceSetResourceBo.setResourceSubType(resource.getSubType());
        resourceSetResourceBo.setScopeModule(ResourceSubTypeEnum.getScopeModuleBySubType(resource.getSubType()));
        resourceSetApi.addResourceSetRelation(resourceSetResourceBo);
    }

    private void initRootUuid(ProtectedResource resource) {
        if (resource.getRootUuid() != null) {
            return;
        }
        String parentUuid = resource.getParentUuid();
        if (parentUuid == null) {
            return;
        }
        if (parentUuid.equals(resource.getUuid())) {
            resource.setParentUuid(null);
            return;
        }
        String rootUuid = protectedResourceMapper.queryRootUuid(parentUuid);
        if (rootUuid == null) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "not found root resource for " + parentUuid);
        }
        resource.setRootUuid(rootUuid);
    }

    /**
     * update resources
     *
     * @param isOverwrite overwrite flag
     * @param resources resources
     */
    @Override
    @Transactional(rollbackFor = Exception.class)
    public void update(boolean isOverwrite, ProtectedResource[] resources) {
        List<ProtectedResource> resourceList = Optional.ofNullable(resources).map(Arrays::asList)
                .map(Objects::requireNonNull).map(ArrayList::new).orElseGet(ArrayList::new);
        // 移除没有指定uuid的非法数据
        resourceList.removeIf(resource -> resource.getUuid() == null);
        if (resourceList.isEmpty()) {
            return;
        }

        List<String> uuidList = resourceList.stream().map(ProtectedResource::getUuid).collect(Collectors.toList());
        Map<Object, ProtectedResourcePo> types = protectedResourceMapper.selectBatchIds(uuidList).stream()
                .map(map -> new AbstractMap.SimpleEntry<>(map.getUuid(), map))
                .collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue));
        resourceList.removeIf(resource -> !types.containsKey(resource.getUuid()));
        resourceList.forEach(resource -> mergeResourceOriginalData(types, resource));
        for (ProtectedResource resource : resourceList) {
            ResourceProvider provider = providerManager.findProvider(ResourceProvider.class, resource, null);
            Set<String> updateCopyProperties = new HashSet<>();
            putUpdateCopyProperties(resource, updateCopyProperties);
            Optional.ofNullable(provider).ifPresent(e -> e.updateCheck(resource));
            update(isOverwrite, resource);
            updateResourceDependency(isOverwrite, resource);
            updateCopyProperties(resource, updateCopyProperties);
        }
    }

    private void putUpdateCopyProperties(ProtectedResource resource, Set<String> updateCopyProperties) {
        List<ProtectedResource> oldResources = resourceService.query(0, 1, Collections.singletonMap("uuid",
            resource.getUuid())).getRecords();
        if (oldResources.isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST);
        }
        if (oldResources.size() > 1) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR);
        }
        if (StringUtils.isNotEmpty(resource.getName())
            && !StringUtils.equals(resource.getName(), oldResources.get(0).getName())) {
            updateCopyProperties.add(UPDATE_COPY_RESOURCE_NAME);
        }
    }

    private void updateCopyProperties(ProtectedResource resource, Set<String> updateCopyProperties) {
        if (updateCopyProperties.contains(UPDATE_COPY_RESOURCE_NAME)) {
            copyServiceSdk.updateCopyResourceName(resource.getName(), resource.getUuid());
        }
    }

    private void updateResourceDependency(boolean isOverwrite, ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = resource.getDependencies();
        if (org.springframework.util.CollectionUtils.isEmpty(dependencies)) {
            return;
        }
        Map<String, ProtectedResource> existResource = findExistResource(resource);
        checkHostTrustFromExistResource(resource, existResource);
        checkAuthorizationAndFillWhenUpdate(resource);
        // delete dependency
        deleteDependency(resource, existResource);
        // save update dependency
        handleResourceDependency(isOverwrite, resource, existResource);
    }

    private void checkAuthorizationAndFillWhenUpdate(ProtectedResource resource) {
        ResourceQueryParams params = new ResourceQueryParams();
        params.setSize(1);
        params.setShouldIgnoreOwner(true);
        params.setShouldLoadEnvironment(false);
        ProtectedResource resourceInDb = query(params).getRecords().stream().findFirst().orElse(null);
        if (resourceInDb == null) {
            log.warn("can not query resource. resource id: {}", resource.getUuid());
            return;
        }
        ProtectedResource combineProtectedResource = ResourceUtil.combineProtectedResource(resource, resourceInDb);
        Map<String, ProtectedResource> existResource = findExistResource(combineProtectedResource, true);
        checkAndFillAuthorizion(resource, existResource);
    }

    private void deleteDependency(ProtectedResource resource, Map<String, ProtectedResource> existResource) {
        Map<String, List<ProtectedResource>> dependencies = resource.getDependencies();
        if (dependencies == null) {
            return;
        }
        Iterator<String> keyIterator = dependencies.keySet().iterator();
        while (keyIterator.hasNext()) {
            String dependencyKey = keyIterator.next();
            for (ProtectedResource protectedResource : dependencies.get(dependencyKey)) {
                if (protectedResource.getUuid() == null) {
                    continue;
                }
                if (dependencyKey.startsWith(UPDATE_DELETE_CONNECTION_PREFIX)) {
                    ProtectedResource update = existResource.get(protectedResource.getUuid());
                    removeResourceExtendInfo(update, recoverDependencyDeleteKey(dependencyKey), resource.getUuid());
                } else if (dependencyKey.startsWith(UPDATE_DELETE_RESOURCE_PREFIX)) {
                    ResourceDeleteParams params = new ResourceDeleteParams();
                    params.setShouldIgnoreDependency(true);
                    params.setResources(new String[]{protectedResource.getUuid()});
                    delete(params);
                } else {
                    deleteDependency(protectedResource, existResource);
                }
            }
            if (shouldDeletedByDependencyKey(dependencyKey)) {
                keyIterator.remove();
            }
        }
    }

    private boolean shouldDeletedByDependencyKey(String dependencyKey) {
        return dependencyKey.startsWith(UPDATE_DELETE_CONNECTION_PREFIX)
                || dependencyKey.startsWith(UPDATE_DELETE_RESOURCE_PREFIX);
    }

    private String recoverDependencyDeleteKey(String dependencyKey) {
        if (StringUtils.isEmpty(dependencyKey) || dependencyKey.length() <= 1) {
            return "";
        }
        return dependencyKey.substring(1);
    }

    private void removeResourceExtendInfo(ProtectedResource resource, String dependencyKey, String citationUUid) {
        if (StringUtils.isEmpty(dependencyKey)) {
            return;
        }
        Map<String, String> extendInfo = resource.getExtendInfo();
        String keyPrefix = ResourceConstants.CITATION + ResourceConstants.CITATION_SEPERATOR + dependencyKey;
        if (extendInfo == null) {
            return;
        }
        Iterator<String> keyIterator = extendInfo.keySet().iterator();
        while (keyIterator.hasNext()) {
            String key = keyIterator.next();
            String value = extendInfo.get(key);
            if (key.startsWith(keyPrefix) && Objects.equals(value, citationUUid)) {
                int count = resourceExtendInfoMapper.delete(new QueryWrapper<ProtectedResourceExtendInfoPo>().lambda()
                        .eq(ProtectedResourceExtendInfoPo::getResourceId, resource.getUuid())
                        .eq(ProtectedResourceExtendInfoPo::getKey, key));
                if (count > 0) {
                    log.debug("truncate {} extend info for {}, key is {}", count, resource.getUuid(), key);
                }
                keyIterator.remove();
            }
        }
    }

    /**
     * update resources
     *
     * @param resources resources
     */
    @Override
    @Transactional
    public void update(ProtectedResource[] resources) {
        update(false, resources);
    }

    /**
     * updateLinkStatus
     *
     * @param uuid uuid
     * @param linkStatus linkStatus
     */
    @Override
    public void updateLinkStatus(String uuid, String linkStatus) {
        repository.updateLinkStatus(uuid, linkStatus);
    }

    private void update(boolean isOverwrite, ProtectedResource resource) {
        ResourceProvider resourceProvider = providerManager.findProvider(ResourceProvider.class, resource, null);
        if (resourceProvider != null && ResourceConstants.SOURCE_TYPE_AUTO_SCAN.equals(resource.getSourceType())
                && resourceProvider.getResourceFeature().isShouldSaveDirectlyWhenScan()) {
            repository.update(resource, isOverwrite);
            return;
        }
        protectedResourceMonitorService.invoke("update", resource, event -> {
            repository.update(event.getResource(), isOverwrite);
            return null;
        });
    }

    private void mergeResourceOriginalData(Map<Object, ProtectedResourcePo> types, ProtectedResource resource) {
        ProtectedResourcePo data = types.get(resource.getUuid());
        if (VerifyUtil.isEmpty(resource.getSubType())) {
            resource.setSubType(data.getSubType());
        }
        String auth = data.getAuth();
        Authentication authentication;
        if (auth != null) {
            authentication = JSONObject.toBean(auth, Authentication.class);
        } else {
            authentication = null;
        }
        decryptService.decrypt(authentication);
        resource.setAuth(mergeAuthentication(new Authentication[]{resource.getAuth(), authentication}));
    }

    /**
     * update source type of protected resource by uuid list
     *
     * @param uuids uuids
     * @param sourceType source type
     */
    @Override
    public void updateSourceType(List<String> uuids, String sourceType) {
        if (VerifyUtil.isEmpty(uuids)) {
            return;
        }
        UpdateWrapper<ProtectedResourcePo> wrapper = new UpdateWrapper<>();
        protectedResourceMapper.update(null, wrapper.in("uuid", uuids).set("source_type", sourceType));
    }

    /**
     * update resource's sub resources
     *
     * @param uuids current resource's uuid
     * @param updateEntry update kv
     */
    @Override
    public void updateSubResource(List<String> uuids, Map<String, Object> updateEntry) {
        if (VerifyUtil.isEmpty(uuids) || MapUtils.isEmpty(updateEntry)) {
            return;
        }
        UpdateWrapper<ProtectedResourcePo> wrapper = new UpdateWrapper<>();
        wrapper.in("parent_uuid", uuids);
        updateEntry.forEach((k, v) -> wrapper.set(k, v));
        protectedResourceMapper.update(null, wrapper);
    }

    /**
     * 直接更新资源到数据库
     * 适用场景：
     * 适用于直接将资源的某些属性同步到数据库中而不用运行入库前其他逻辑的更新情况，
     * 只需要保证待更新的属性能够入库成功。
     *
     * @param resources 待更新的资源
     */
    @Override
    @Transactional
    public void updateSourceDirectly(List<ProtectedResource> resources) {
        if (resources == null || resources.isEmpty()) {
            return;
        }
        for (ProtectedResource resource : resources) {
            // 移除没有指定uuid的非法数据
            if (resource.getUuid() == null) {
                continue;
            }
            // 直接更新数据库
            repository.update(resource, false);
        }
    }

    /**
     * update or insert resources
     *
     * @param isOverwrite overwrite flag
     * @param resources resources
     * @return resource uuids
     */
    @Override
    @Transactional
    public ResourceUpsertRes upsert(boolean isOverwrite, ProtectedResource... resources) {
        List<ProtectedResource> resourceList = CollectionUtils.nonNullList(resources);
        // 过滤掉资源扩展字段中scanSkipUpdateAndInsert为true的资源
        List<ProtectedResource> needUpdateOrAddResources = resourceList.stream()
                .filter(resource -> isNeedUpdateAndInsert(resource)).collect(Collectors.toList());
        List<String> uuidList = needUpdateOrAddResources.stream().map(ProtectedResource::getUuid)
                .filter(Objects::nonNull).collect(Collectors.toList());
        List<String> existResourceUuids;
        if (uuidList.isEmpty()) {
            existResourceUuids = Collections.emptyList();
        } else {
            // 这边超过数额需要把过滤逻辑放在这边进行
            existResourceUuids = protectedResourceMapper.queryRelatedResourceUuids(uuidList);
        }

        List<ProtectedResource> newResourcesList = needUpdateOrAddResources.stream()
                .filter(resource -> !existResourceUuids.contains(resource.getUuid()))
                .collect(Collectors.toList());

        // 创建新增插件类型资源，规格无需校验
        ProtectedResource[] newPluginResources = newResourcesList.stream()
                .filter(resource -> StringUtils.equals(resource.getType(), ResourceTypeEnum.PLUGIN.getType()))
                .toArray(ProtectedResource[]::new);
        String[] pluginUuids = create(newPluginResources);

        // 创建新增非插件类型资源，需与最大规格校验
        ProtectedResource[] newResources = newResourcesList.stream()
                .filter(resource -> !StringUtils.equals(resource.getType(), ResourceTypeEnum.PLUGIN.getType()))
                .toArray(ProtectedResource[]::new);
        ProtectedResource[] limitedNewResources = limitResourceCount(newResources);
        String[] newUuids = create(limitedNewResources);

        // 总共新增的资源
        String[] uuids = ArrayUtils.addAll(pluginUuids, newUuids);

        // 更新已有资源
        ProtectedResource[] oldResources = needUpdateOrAddResources.stream()
                .filter(resource -> existResourceUuids.contains(resource.getUuid())).toArray(ProtectedResource[]::new);
        update(isOverwrite, oldResources);

        ResourceUpsertRes resourceUpsertRes = new ResourceUpsertRes();
        resourceUpsertRes.setIncreaseResourceUuids(uuids);
        resourceUpsertRes.setOverLimit(limitedNewResources.length < newResources.length);
        return resourceUpsertRes;
    }

    private boolean isNeedUpdateAndInsert(ProtectedResource resource) {
        return Optional.ofNullable(resource.getExtendInfoByKey(SCAN_SKIP_UPDATE_AND_INSERT))
                .map(e -> !Boolean.TRUE.toString().equals(e))
                .orElse(true);
    }

    private ProtectedResource[] limitResourceCount(ProtectedResource[] newResources) {
        int maxSize = maxResourceNum;
        if (deployTypeService.isHyperDetectDeployType()) {
            maxSize = MAX_DORADO_RESOURCE_NUM;
        }
        if (maxSize == -1) {
            return newResources;
        }
        QueryWrapper queryWrapper = new QueryWrapper();
        queryWrapper.ne("type", ResourceTypeEnum.PLUGIN.getType());
        int currentSize = protectedResourceMapper.selectCount(queryWrapper).intValue();
        log.info("Limit resource size. to add size:{}, currentSize: {}, limit: {}", newResources.length, currentSize,
            maxSize);
        if (newResources.length <= maxSize - currentSize) {
            return newResources;
        }
        return Arrays.copyOf(newResources, maxSize - currentSize);
    }

    /**
     * merge auth param from source to target
     *
     * @param authentications authentications
     * @return new auth object
     */
    @Override
    public Authentication mergeAuthentication(Authentication[] authentications) {
        Authentication[] items = Optional.ofNullable(authentications).map(Arrays::asList).map(Collection::stream)
                .orElse(Stream.empty()).filter(Objects::nonNull).toArray(Authentication[]::new);
        if (items.length == 0) {
            return null;
        }
        Authentication auth = new Authentication();
        Integer authType = getAnyValue(Authentication::getAuthType, items);
        if (authType == null) {
            throw new LegoCheckedException(ErrorCodeConstant.ERR_PARAM, "auth type is invalid");
        }
        auth.setAuthType(authType);
        auth.setAuthKey(getAnyValue(Authentication::getAuthKey, items));
        auth.setAuthPwd(getAnyValue(Authentication::getAuthPwd, items));
        Map<String, String> extendInfo = getMergedValue(
                Stream.of(items).map(item -> Optional.ofNullable(item).map(Authentication::getExtendInfo).orElse(null))
                        .collect(Collectors.toList()));
        auth.setExtendInfo(extendInfo);
        return auth;
    }

    /**
     * merge protected resource
     *
     * @param protectedResource protectedResource
     * @return resource
     */
    @Override
    public ProtectedResource mergeProtectedResource(ProtectedResource protectedResource) {
        return getResourceById(protectedResource.getUuid())
                .map(resource -> replenishEnvironment(
                        ResourceUtil.merge(ProtectedResource.class, resource, protectedResource, true)))
                .orElseGet(() -> replenishEnvironment(protectedResource));
    }

    /**
     * replenish Environment
     *
     * @param resource resource
     * @return resource
     */
    @Override
    public ProtectedResource replenishEnvironment(ProtectedResource resource) {
        ProtectedEnvironment originEnvironment = resource.getEnvironment();
        // 优先使用root uuid进行查询补全信息。在root uuid缺失的情况下，再使用parent uuid进行信息补全。
        replenishEnvironmentByRootUuid(resource).orElseGet(() -> replenishEnvironmentByParentUuid(resource));
        if (originEnvironment != null) {
            resource.setEnvironment(ProxyFactory.get(ProtectedEnvironment.class)
                    .create(Arrays.asList(originEnvironment, resource.getEnvironment())));
        }
        ProtectedEnvironment environment = resource.getEnvironment();
        if (environment != null) {
            resource.setRootUuid(Optional.ofNullable(resource.getRootUuid()).orElse(environment.getUuid()));
        }
        return resource;
    }

    @Override
    public void createProtectedResourceScanTask(String resId, String userId) {
        Optional<ProtectedResource> resourceOptional = getBasicResourceById(resId);
        ProtectedResource protectedResource = resourceOptional.orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected resource is not exists!"));
        if (deployTypeService.isHyperDetectDeployType()) {
            protectedResource.setName(ResourceConstant.LOCAL_STORAGE_FILE_SYSTEM);
        }
        jobScheduleService.createJobSchedule(ResourceConstant.MANUAL_SCAN_RESOURCE, protectedResource, userId,
                new JSONObject().set(ResourceConstant.RES_ID, resId));
    }

    @Override
    public List<String> scanProtectedResource(ProtectedResource resource, boolean isStrict,
            Consumer<ResourceScanDto> afterScan) {
        if (resource instanceof ProtectedEnvironment) {
            String linkStatus = EnvironmentLinkStatusHelper
                    .getLinkStatusAdaptMultiCluster((ProtectedEnvironment) resource);
            if (LinkStatusEnum.OFFLINE.getStatus().toString().equals(linkStatus)) {
                log.warn("The environment: {} is offline, no need to scan.", resource.getUuid());
                throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                        "The target environment is offline");
            }
        }

        AtomicReference<List<String>> ref = new AtomicReference<>();
        tryLockAndRun(() -> doExecuteScanProtectedResource(ref, resource, afterScan), resource.getUuid(),
                ResourceConstant.RESOURCE_SCAN_LOCK_WAIT_TIME, isStrict);
        return ref.get();
    }

    private void doExecuteScanProtectedResource(AtomicReference<List<String>> ref, ProtectedResource resource,
            Consumer<ResourceScanDto> afterScan) {
        ResourceScanDto resourceScanDto = new ResourceScanDto();
        resourceScanDto.setResource(resource);
        try {
            ref.set(executeScanProtectedResource(resource));
        } catch (Exception e) {
            resourceScanDto.setException(e);
            log.error("execute scan protected resource failed. resourceId is {}", resource.getUuid(),
                    ExceptionUtil.getErrorMessage(e));
            throw LegoCheckedException.cast(e);
        } finally {
            if (afterScan != null) {
                afterScan.accept(resourceScanDto);
            }
        }
    }

    private void tryLockAndRun(Runnable runnable, String resId, long time, boolean isStrict) {
        Lock lock = lockService.createSQLDistributeLock(ResourceConstant.RESOURCE_LOCK_KEY + resId);
        boolean isDone = lock.tryLockAndRun(time, TimeUnit.SECONDS, runnable);
        if (isStrict && !isDone) {
            throw new LockNotObtainedException(deployTypeService.isCyberEngine()
                ? ResourceLockErrorCode.OCEAN_CYBER_RESOURCE_ALREADY_LOCKED
                : ResourceLockErrorCode.RESOURCE_ALREADY_LOCKED, new String[] {resId},
                    "resource is locked. id: " + resId);
        } else {
            log.debug("try lock and run: {}, strict: {}, resId: {}", isDone, isStrict, resId);
        }
        if (!isDone) {
            log.warn("can not acquire lock and run, resId: {}", resId);
        }
    }

    /**
     * 扫描受保护资源
     *
     * @param resource 受保护资源
     * @return 新增的子资源id列表
     */
    public List<String> executeScanProtectedResource(ProtectedResource resource) {
        Timer timer = setTimer(resource.getUuid());
        // 再次确认资源是否还存在
        String resourceId = resource.getUuid();
        getBasicResourceById(resourceId).orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected resource is not exists!"));

        String subType = resource.getSubType();
        log.info("Scan resource name is {}, uuid is {} and sub type is {}", resource.getName(), resourceId, subType);
        List<ProtectedResource> protectedResourceList;
        try {
            protectedResourceList = doScanResources(resource, subType);
        } catch (Throwable e) {
            log.error("resource({}) scan failed", resourceId, e);
            throw LegoCheckedException.cast(e);
        }
        ResourceProvider resourceProvider = providerManager.findProvider(ResourceProvider.class, resource, null);
        if (resourceProvider != null) {
            ExecuteScanRes executeScanRes = resourceProvider.afterScanHandleResource(resource, protectedResourceList);
            if (executeScanRes != null && executeScanRes.isEndExecute()) {
                return executeScanRes.getScanNewResourceUuids();
            }
        }

        String[] effectiveResourceUuids = protectedResourceList.stream().map(ProtectedResource::getUuid)
                .filter(Objects::nonNull).toArray(String[]::new);
        Set<String> redundantResourceUuids;
        if (effectiveResourceUuids.length >= 30000 && deployTypeService.isHyperDetectDeployType()) {
            redundantResourceUuids = queryAndFilter(resourceId, effectiveResourceUuids);
        } else {
            redundantResourceUuids = queryRelatedResourceUuids(resourceId, effectiveResourceUuids);
        }
        redundantResourceUuids.remove(resourceId);
        // 主存不涉及注册资源
        if (!deployTypeService.isHyperDetectDeployType()) {
            removeResourceUuidWithRegister(redundantResourceUuids);
        }
        if (!redundantResourceUuids.isEmpty()) {
            log.info("delete {} child resources of {}(type: {}, id: {})", redundantResourceUuids.size(),
                    resource.getName(), subType, resourceId);
            deleteResources(redundantResourceUuids);
        }
        log.info("executeScanProtectedResource success, protectedResourceList size: {}", protectedResourceList.size());
        handleUpdateDependencyHostInfo(protectedResourceList);
        if (deployTypeService.isHyperDetectDeployType()) {
            sendResourceInfosToDee(protectedResourceList, resourceId);
        }
        // 批量新增
        return upsertResources(protectedResourceList, resource, timer);
    }

    private Timer setTimer(String resourceId) {
        Timer timer = new Timer();
        if (deployTypeService.isHyperDetectDeployType()) {
            List<JobBo> jobBos = resourceScanService.queryManualScanRunningJobByResId(resourceId);
            if (jobBos.size() > 0) {
                log.info("start deploy timer, resourceId:{}", resourceId);
                startTimer(timer, jobBos);
            }
        }
        return timer;
    }

    private void startTimer(Timer timer, List<JobBo> jobBos) {
        timer.schedule(new TimerTask() {
            int count = 0;
            @Override
            public void run() {
                if (count < TIMES) {
                    log.info("keep scan job, jobid:{}", jobBos.get(0).getJobId());
                    JobBo jobBo = jobBos.get(0);
                    UpdateJobRequest updateJobRequest = new UpdateJobRequest();
                    updateJobRequest.setProgress(Optional.ofNullable(jobBo.getProgress()).orElse(0));
                    jobService.updateJob(jobBo.getJobId(), updateJobRequest);
                } else {
                    timer.cancel();
                }
                count++;
            }
        }, 0, TIMER_INTERVAL);
    }

    private void deleteResources(Set<String> redundantResourceUuids) {
        // 删除受保护资源时，告警
        resourceAlarmService.alarmDeleteProtectedResource(redundantResourceUuids);
        // 做批量删除
        if (redundantResourceUuids.size() >= 30000 && deployTypeService.isHyperDetectDeployType()) {
            List<List<String>> partitionedList = ListUtils.partition(
                new ArrayList<>(redundantResourceUuids), PARTITION_SIZE);
            partitionedList.forEach(lists ->
                delete(new ResourceDeleteParams(true, false, lists.toArray(new String[0]))));
        } else {
            delete(new ResourceDeleteParams(true, false, redundantResourceUuids.toArray(new String[0])));
        }
    }

    private List<String> upsertResources(List<ProtectedResource> protectedResourceList, ProtectedResource resource,
        Timer timer) {
        ResourceUpsertRes upsertRes;
        if (protectedResourceList.size() >= 30000 && deployTypeService.isHyperDetectDeployType()) {
            List<List<ProtectedResource>> partitionedResources = ListUtils.partition(
                new ArrayList<>(protectedResourceList), PARTITION_SIZE);
            List<String> upsertResourses = new ArrayList<>();
            boolean isOverLimit = false;
            for (List<ProtectedResource> resources : partitionedResources) {
                ResourceUpsertRes res = upsert(resource, resources);
                upsertResourses.addAll(new ArrayList<>(Arrays.asList(res.getIncreaseResourceUuids())));
                isOverLimit = isOverLimit || res.isOverLimit();
            }
            upsertRes = new ResourceUpsertRes();
            upsertRes.setOverLimit(isOverLimit);
            upsertRes.setIncreaseResourceUuids(upsertResourses.toArray(new String[upsertResourses.size()]));
        } else {
            upsertRes = upsert(resource, protectedResourceList);
        }

        // 更新保护
        checkResSubtypeAndUpdateProtectStatus(resource, protectedResourceList);

        List<String> increaseResourceUuidList = Optional.ofNullable(upsertRes.getIncreaseResourceUuids())
            .map(Arrays::asList).orElse(Collections.emptyList());
        if (upsertRes.isOverLimit()) {
            updateExceedLimitJobLog(resource, increaseResourceUuidList);
        }
        timer.cancel();
        return increaseResourceUuidList;
    }

    private void checkResSubtypeAndUpdateProtectStatus(ProtectedResource resource,
        List<ProtectedResource> protectedResourceList) {
        // 只有云和虚拟化插件需要适配，因为FC和FC ONE两款插件的类型和子类型是相反的，所以要分别判断
        if (!UPDATE_PROTECT_TYPE_LIST.contains(resource.getType())
            && !(FC_AND_FC_ONE_TYPE_LIST.contains(resource.getSubType())
            && ResourceTypeEnum.PLATFORM.getType().equals(resource.getType()))) {
            return;
        }
        checkProtectedResourceTypeAndUpdateProtectStatus(protectedResourceList);
    }

    private void checkProtectedResourceTypeAndUpdateProtectStatus(List<ProtectedResource> protectedResourceList) {
        for (ProtectedResource protectedResource : protectedResourceList) {
            // 这里部分插件protectedResource中没有parentUuid字段，因此需要去数据库中查询详细资源信息后获取parentUuid的值
            Optional<ProtectedResource> protectedResOpt = resourceService.getResourceById(protectedResource.getUuid());
            if (!protectedResOpt.isPresent()) {
                continue;
            }
            ProtectedResource protectedRes = protectedResOpt.get();
            boolean isVmOrCloudServer = NEED_UPDATE_PROTECT_STATUS_TYPE_LIST.contains(protectedRes.getSubType())
                || (FC_AND_FC_ONE_TYPE_LIST.contains(protectedRes.getSubType())
                && ResourceTypeEnum.VM.getType().equals(protectedRes.getType()));
            if (!isVmOrCloudServer) {
                // 只校验虚拟机或云服务器，List中VM是FC或FC ONE，LIST中其他的值对应的是其他虚拟化和云平台插件
                // HCS只需考虑线下场景HCSContainer，线上场景因为资源只有一层云服务器，因此不考虑HCS线上场景HcsEnvOp
                // 阿里云云服务费SLA的变化只根据其父资源可用区的SLA的配置而变化，不根据资源集的SLA的配置而变化，云服务器和可用区无父子关系
                continue;
            }
            String parentUuid = protectedRes.getParentUuid();
            Optional<ProtectedResource> parentResourceOpt = resourceService.getResourceById(parentUuid);
            if (parentResourceOpt.isPresent()) {
                ProtectedResource parentResource = parentResourceOpt.get();
                if (parentResource.getProtectionStatus().equals(ProtectionStatusEnum.UNPROTECTED.getType())) {
                    continue;
                }
                updateProtectStatus(parentResource, protectedRes);
            }
        }
    }

    private void updateProtectStatus(ProtectedResource parentResource, ProtectedResource protectedRes) {
        if (!isMeetsFilterCondition(parentResource, protectedRes)) {
            // 不满足过滤条件
            updateProtectStatusWhenNotMeetsFilterCondition(parentResource, protectedRes);
        } else {
            // 满足过滤条件
            updateProtectStatusWhenMeetsFilterCondition(parentResource, protectedRes);
        }
    }

    private void updateProtectStatusWhenMeetsFilterCondition(ProtectedResource parentResource,
        ProtectedResource protectedRes) {
        String parentSlaId = parentResource.getProtectedObject().getSlaId();
        String parentSlaName = parentResource.getProtectedObject().getSlaName();
        Map<String, Object> parentExtendInfo = parentResource.getProtectedObject().getExtParameters();
        boolean isOverwrite = (boolean) parentExtendInfo.get(OVERWRITE);
        String bindingPolicy = parentExtendInfo.get(BINDING_POLICY).toString();
        int protectStatus = protectedRes.getProtectionStatus();
        switch (ProtectionStatusEnum.getStatus(protectStatus)) {
            case PROTECTED:
                String slaId = protectedRes.getProtectedObject().getSlaId();
                if (isOverwrite && !slaId.equals(parentSlaId)) {
                    modifyProtectionForRes(protectedRes, parentSlaId, parentSlaName, parentResource, parentExtendInfo);
                }
                break;
            case UNPROTECTED:
                if (bindingPolicy.contains(APPLY_TO_ALL)) {
                    createProtectionForRes(protectedRes, parentSlaId, parentSlaName, parentResource, parentExtendInfo);
                }
                break;
            case PROTECTING:
                log.warn("Can not change protection(PROTECTING) of resources:{}.", protectedRes.getUuid());
                break;
            default:
                log.error("Unknown protection status:{} of resource:{}.", protectStatus, protectedRes.getUuid());
                break;
        }
    }

    private void createProtectionForRes(ProtectedResource protectedRes, String parentSlaId, String parentSlaName,
        ProtectedResource parentResource, Map<String, Object> parentExtendInfo) {
        try {
            String userId = UserConstants.SYS_ADMIN_USER_ID;
            ProtectionCreationDto protectionCreationRequest = generateProtectionCreation(parentSlaId,
                protectedRes.getUuid(), parentExtendInfo);
            List<String> jobIds = protectObjectRestApi.createProtectedObject(userId, protectionCreationRequest);
            String[] eventParams = setEventParams(parentResource, protectedRes, parentSlaName);
            sendEvent(CommonOperationCode.AUTO_SCAN_AND_CREATE_PROTECT, eventParams, protectedRes.getUuid());
            log.info("Create protection job: {} of resource: {} and sla: (id:{}, name:{}) success.",
                jobIds, protectedRes.getUuid(), parentSlaId, parentSlaName);
        } catch (LegoCheckedException ex) {
            log.error("Create protection job of resource: {} and sla: (id:{}, name:{}) failed.",
                protectedRes.getUuid(), parentSlaId, parentSlaName, ExceptionUtil.getErrorMessage(ex));
        }
    }

    private void modifyProtectionForRes(ProtectedResource protectedRes, String parentSlaId, String parentSlaName,
        ProtectedResource parentResource, Map<String, Object> parentExtendInfo) {
        try {
            String userId = UserConstants.SYS_ADMIN_USER_ID;
            ProtectionModifyDto protectionModifyRequest = generateProtectionModifyReq(parentSlaId,
                protectedRes.getUuid(), protectedRes.getProtectedObject(), parentExtendInfo);
            String jobId = protectObjectRestApi.modifyProtectedObject(userId, protectionModifyRequest);
            String[] eventParams = setEventParams(parentResource, protectedRes, parentSlaName);
            sendEvent(CommonOperationCode.AUTO_SCAN_AND_MODIFY_PROTECT, eventParams, protectedRes.getUuid());
            log.info("Modify protection job: {} of resource: {} and sla: (id:{}, name:{}) success.",
                jobId, protectedRes.getUuid(), parentSlaId, parentSlaName);
        } catch (LegoCheckedException ex) {
            log.error("Modify protection job of resource: {} and sla: (id:{}, name:{}) failed.",
                protectedRes.getUuid(), parentSlaId, parentSlaName, ExceptionUtil.getErrorMessage(ex));
        }
    }

    private String[] setEventParams(ProtectedResource parentResource, ProtectedResource protectedRes, String slaName) {
        return new String[] {parentResource.getEnvironment().getName(), protectedRes.getUuid(),
            protectedRes.getName(), "--", slaName, "--"};
    }

    private void updateProtectStatusWhenNotMeetsFilterCondition(ProtectedResource parentResource,
        ProtectedResource protectedRes) {
        if (protectedRes.getProtectionStatus().equals(ProtectionStatusEnum.PROTECTED.getType())) {
            // 当不满足被保护的条件，如果有保护，移除保护
            deleteProtectionForRes(protectedRes, parentResource);
        }
    }

    private void deleteProtectionForRes(ProtectedResource protectedRes, ProtectedResource parentResource) {
        try {
            ProtectionBatchOperationReq deleteGroupSubSourceReq = new ProtectionBatchOperationReq();
            deleteGroupSubSourceReq.setIsResourceGroup(false);
            List<String> deleteResourceIds = new ArrayList<>();
            deleteResourceIds.add(protectedRes.getUuid());
            deleteGroupSubSourceReq.setResourceIds(deleteResourceIds);
            protectObjectRestApi.deleteProtectedObjects(deleteGroupSubSourceReq);
            String[] eventParams = setEventParams(parentResource, protectedRes,
                protectedRes.getProtectedObject().getSlaName());
            sendEvent(CommonOperationCode.AUTO_SCAN_AND_REMOVE_PROTECT, eventParams, protectedRes.getUuid());
            log.info("Remove protection of resource: {} success.", protectedRes.getUuid());
        } catch (LegoCheckedException ex) {
            log.error("Remove protection of resource: {} failed.", protectedRes.getUuid(),
                ExceptionUtil.getErrorMessage(ex));
        }
    }

    private void sendEvent(String eventId, String[] eventParams, String resourceId) {
        LegoInternalEvent event = new LegoInternalEvent();
        event.setResourceId(resourceId);
        event.setSourceType(EventTarget.PROTECTION);
        event.setSourceId(eventId);
        event.setEventLevel(FaultEnum.AlarmSeverity.INFO);
        event.setEventId(eventId);
        event.setEventTime(Instant.now().getEpochSecond());
        event.setEventSequence(Instant.now().getNano());
        event.setIsSuccess(true);
        event.setEventParam(eventParams);
        event.setUserId(UserConstants.SYS_ADMIN_USER_ID);
        operationLogService.sendEvent(event);
    }

    private boolean isMeetsFilterCondition(ProtectedResource parentResource, ProtectedResource protectedRes) {
        Map<String, Object> parentExtendInfo = parentResource.getProtectedObject().getExtParameters();
        Optional<Object> resourceFiltersObjOpt = Optional.ofNullable(parentExtendInfo.get(RESOURCE_FILTERS));
        Object resourceFiltersObj = resourceFiltersObjOpt.orElse(null);
        List<ResourceFilterParam> resourceFilterParams = new ArrayList<>();
        if (resourceFiltersObj instanceof List) {
            List<?> resourceFilters = (List<?>) resourceFiltersObj;
            resourceFilterParams = JSON.parseObject(JSON.toJSONString(resourceFilters),
                new TypeReference<List<ResourceFilterParam>>() {});
        } else {
            log.warn("ParentResource:{} do not have the 'resource_filters' value.", parentResource.getUuid());
        }
        Object resourceTagFiltersObj = parentExtendInfo.get(RESOURCE_TAG_FILTERS);
        List<ResourceFilterParam> resourceTagFilterParams = new ArrayList<>();
        if (resourceTagFiltersObj instanceof List) {
            List<?> resourceFilters = (List<?>) resourceTagFiltersObj;
            resourceTagFilterParams = JSON.parseObject(JSON.toJSONString(resourceFilters),
                new TypeReference<List<ResourceFilterParam>>() {});
        } else {
            log.warn("ParentResource:{} do not have the 'resource_tag_filters' value.", parentResource.getUuid());
        }
        ResourceFilterConditionParam resourceFilterConditionParam = new ResourceFilterConditionParam();
        resourceFilterConditionParam.setResourceFilters(resourceFilterParams);
        resourceFilterConditionParam.setResourceTagFilters(resourceTagFilterParams);
        return ResourceFilterUtil.isResourceMeetsFilterConditions(protectedRes, resourceFilterConditionParam);
    }

    private ProtectionCreationDto generateProtectionCreation(String slaId, String resourceId,
        Map<String, Object> parentExtendInfo) {
        ProtectionCreationDto creationDto = new ProtectionCreationDto();
        creationDto.setSlaId(slaId);
        creationDto.setResources(buildProtectionResources(resourceId));
        Map<String, Object> parameters = new HashMap<>();
        parameters.put(DISK_INFO, new ArrayList<>());
        Map<String, Object> extParameters = setExtParameters(parentExtendInfo, parameters);
        creationDto.setExtParameters(extParameters);
        return creationDto;
    }

    private static List<ProtectionResourceDto> buildProtectionResources(String resourceId) {
        // 对云主机保护，无资源过滤规则
        ProtectionResourceDto resourceDto = new ProtectionResourceDto();
        resourceDto.setResourceId(resourceId);
        return Collections.singletonList(resourceDto);
    }

    private ProtectionModifyDto generateProtectionModifyReq(String parentSlaId, String resourceId,
        ProtectedObject oldProtectedObject, Map<String, Object> parentExtendInfo) {
        ProtectionModifyDto protection = new ProtectionModifyDto();
        protection.setSlaId(parentSlaId);
        protection.setResourceId(resourceId);
        protection.setIsResourceGroup(false);
        protection.setIsGroupSubResource(false);
        protection.setResourceGroupId("");
        Map<String, Object> parameters = oldProtectedObject.getExtParameters();
        Map<String, Object> extParameters = setExtParameters(parentExtendInfo, parameters);
        protection.setExtParameters(extParameters);
        return protection;
    }

    private Map<String, Object> setExtParameters(Map<String, Object> parentExtendInfo, Map<String, Object> parameters) {
        parameters.put(PRE_SCRIPT, parentExtendInfo.get(PRE_SCRIPT));
        parameters.put(POST_SCRIPT, parentExtendInfo.get(POST_SCRIPT));
        parameters.put(ENABLE_SECURITY_ARCHIVE, parentExtendInfo.get(ENABLE_SECURITY_ARCHIVE));
        parameters.put(WORM_SWITCH, parentExtendInfo.get(WORM_SWITCH));
        parameters.put(AGENTS, parentExtendInfo.get(AGENTS));
        parameters.put(SNAP_DELETE_SPEED, parentExtendInfo.get(SNAP_DELETE_SPEED));
        parameters.put(IS_CONSISTENT, parentExtendInfo.get(IS_CONSISTENT));
        parameters.put(BACKUP_RES_AUTO_INDEX, parentExtendInfo.get(BACKUP_RES_AUTO_INDEX));
        parameters.put(ARCHIVE_RES_AUTO_INDEX, parentExtendInfo.get(ARCHIVE_RES_AUTO_INDEX));
        return parameters;
    }

    private void sendResourceInfosToDee(List<ProtectedResource> protectedResourceList, String resourceId) {
        List<ProtectedResource> fileSystemList = protectedResourceList.stream()
            .filter(info -> FILE_SYSTEM.equals(info.getSubType()))
            .collect(Collectors.toList());
        sendFileSystemToDee(resourceId, fileSystemList);
        sendLunInfoToDee(protectedResourceList);
    }

    private Set<String> queryAndFilter(String resourceId, String[] effectiveResourceIds) {
        Set<String> redundantResources = queryRelatedResourceUuids(resourceId, new String[]{});
        for (String id : effectiveResourceIds) {
            redundantResources.remove(id);
        }
        return redundantResources;
    }

    private ResourceInfo getResourceInfo(ProtectedResource protectedResource) {
        ResourceInfo resourceInfo = new ResourceInfo();
        resourceInfo.setFsName(protectedResource.getName());
        Map<String, String> fileSystemExtendInfo = protectedResource.getExtendInfo();
        resourceInfo.setVstoreId(fileSystemExtendInfo.get("tenantId"));
        resourceInfo.setVstoreName(fileSystemExtendInfo.get("tenantName"));
        resourceInfo.setResourceId(fileSystemExtendInfo.get("fileSystemId"));
        return resourceInfo;
    }

    private void sendFileSystemToDee(String deviceId, List<ProtectedResource> protectedResourceList) {
        UpdateFileSystemRequest updateFileSystemRequest = new UpdateFileSystemRequest();
        updateFileSystemRequest.setDeviceId(deviceId);
        List<ResourceInfo> resourceInfos = protectedResourceList.stream()
            .map(this::getResourceInfo)
            .collect(Collectors.toList());
        updateFileSystemRequest.setResourceInfos(resourceInfos);
        log.info("send filesystem info to dee.");
        antiRansomwareDeviceApi.updateFileSystem(updateFileSystemRequest);
    }

    private void sendLunInfoToDee(List<ProtectedResource> protectedResourceList) {
        List<LunInfo> lunInfoList = protectedResourceList.stream()
            .filter(protectedResource -> (protectedResource.getSubType().equals("LUN"))).map((protectedResource) -> {
                LunInfo lunInfo = new LunInfo();
                lunInfo.setResId(protectedResource.getUuid());
                lunInfo.setLunId(protectedResource.getExtendInfoByKey("lunId"));
                lunInfo.setLunName(protectedResource.getName());
                lunInfo.setVstoreId(protectedResource.getExtendInfo().get("tenantId"));
                lunInfo.setVstoreName(protectedResource.getExtendInfo().get("tenantName"));

                return lunInfo;
            }).collect(Collectors.toList());
        UpdateLunInfoReq updateLunInfoReq = new UpdateLunInfoReq(lunInfoList);
        antiRansomwareDeviceApi.updateLunInfo(updateLunInfoReq);
    }

    private void handleUpdateDependencyHostInfo(List<ProtectedResource> protectedResourceList) {
        for (ProtectedResource protectedResource : protectedResourceList) {
            ResourceProvider provider = providerManager.findProvider(ResourceProvider.class, protectedResource, null);
            Boolean shouldUpdateHostInfo = Optional.ofNullable(provider).map(ResourceProvider::getResourceFeature)
                    .map(ResourceFeature::isShouldUpdateDependencyHostInfoWhenScan).orElse(true);
            if (!shouldUpdateHostInfo) {
                ResourceUtil.setUpdateDependencyHostInfoNull(protectedResource);
            }
        }
    }

    private List<ProtectedResource> doScanResources(ProtectedResource resource, String subType) {
        List<ProtectedResource> protectedResourceList;
        try {
            if (resource instanceof ProtectedEnvironment) {
                EnvironmentProvider provider = providerManager.findProvider(EnvironmentProvider.class, subType);
                protectedResourceList = provider.scan((ProtectedEnvironment) resource);
            } else {
                ResourceProvider provider = providerManager.findProvider(ResourceProvider.class, resource, null);
                if (Objects.nonNull(provider)) {
                    protectedResourceList = provider.scan(resource);
                } else {
                    protectedResourceList = Collections.emptyList();
                }
            }
        } catch (DataProtectionRejectException e) {
            throw new LegoCheckedException(e.getErrorCode(), e.getParameters(), e);
        }
        // 防呆，避免插件给返回的所有资源中设置了同一个Auth对象，导致入库前被重复加密
        protectedResourceList.forEach(res -> {
            Authentication newAuth = BeanTools.copy(res.getAuth(), Authentication::new);
            res.setAuth(newAuth);
        });
        return protectedResourceList;
    }

    private void removeResourceUuidWithRegister(Set<String> redundantResourceUuids) {
        List<String> registerUuids = repository.queryExistResourceUuidBySourceType(
                new ArrayList<>(redundantResourceUuids), ResourceConstants.SOURCE_TYPE_REGISTER);
        registerUuids.forEach(redundantResourceUuids::remove);
    }

    private ResourceUpsertRes upsert(ProtectedResource resource, List<ProtectedResource> protectedResourceList) {
        protectedResourceList.forEach(e -> {
            if (StringUtils.isEmpty(e.getSourceType())) {
                e.setSourceType(ResourceConstants.SOURCE_TYPE_AUTO_SCAN);
            }
        });

        ResourceUpsertRes upsertRes = upsert(protectedResourceList.toArray(new ProtectedResource[0]));
        List<String> increaseResourceUuidList = Optional.ofNullable(upsertRes.getIncreaseResourceUuids())
                .map(Arrays::asList).orElse(Collections.emptyList());
        if (!increaseResourceUuidList.isEmpty()) {
            String subType = resource.getSubType();
            String environmentId = resource.getUuid();
            log.info("increase {} child resources of {}(type: {}, id: {})", increaseResourceUuidList.size(),
                    resource.getName(), subType, environmentId);
            updateSourceType(increaseResourceUuidList, "autoscan");
        }
        log.info("upsert success,resource id:{}", resource.getUuid());
        return upsertRes;
    }

    private void updateExceedLimitJobLog(ProtectedResource resource, List<String> increaseResourceUuidList) {
        List<JobBo> jobBos = resourceScanService.queryManualScanRunningJobByResId(resource.getUuid());
        if (jobBos.size() > 0) {
            JobBo jobBo = jobBos.get(0);
            JobLogBo jobLogBo = new JobLogBo();
            jobLogBo.setJobId(jobBo.getJobId());
            jobLogBo.setStartTime(System.currentTimeMillis());
            jobLogBo.setLogInfo(SCAN_RESOURCE_EXCEED_LIMIT_LABEL);
            jobLogBo.setLevel(JobLogLevelEnum.WARNING.getValue());
            jobLogBo.setLogInfoParam(Collections.singletonList(String.valueOf(increaseResourceUuidList.size())));
            UpdateJobRequest request = new UpdateJobRequest();
            request.setJobLogs(Collections.singletonList(jobLogBo));
            jobService.updateJob(jobBo.getJobId(), request);
            log.info("exceed limit to update job. job id: {}, res id: {}", jobBo.getJobId(), resource.getUuid());
        }
    }

    private Optional<ProtectedResource> replenishEnvironmentByRootUuid(ProtectedResource resource) {
        // 根据root uuid查询受保护环境
        // 针对有dependency依赖的资源，框架目前不去补全dependency信息
        Optional<ProtectedEnvironment> environmentOptional = getBasicResourceById(resource.getRootUuid())
                .filter(ProtectedEnvironment.class::isInstance).map(ProtectedEnvironment.class::cast);
        environmentOptional.ifPresent(resource::setEnvironment);
        return environmentOptional.map(environment -> resource);
    }

    private ProtectedResource replenishEnvironmentByParentUuid(ProtectedResource protectedResource) {
        String parentUuid = protectedResource.getParentUuid();
        Optional<ProtectedResource> parentResourceOptional = getBasicResourceById(true, true, parentUuid);
        if (!parentResourceOptional.isPresent()) {
            // 根据parent uuid没有查询到资源的情况下，不进行信息补全
            return protectedResource;
        }
        ProtectedResource parentResource = parentResourceOptional.get();
        if (parentResource instanceof ProtectedEnvironment) {
            // 如果父资源本身就是受保护环境，则直接补全
            protectedResource.setEnvironment((ProtectedEnvironment) parentResource);
        } else {
            // 如果父资源本身不是受保护环境，则使用其受保护环境信息进行补全
            protectedResource.setEnvironment(parentResource.getEnvironment());
        }
        return protectedResource;
    }

    private <T> T getAnyValue(Function<Authentication, T> function, Authentication... authentications) {
        for (Authentication authentication : authentications) {
            if (authentication != null) {
                T value = function.apply(authentication);
                if (value != null) {
                    return value;
                }
            }
        }
        return null;
    }

    private Map<String, String> getMergedValue(List<Map<String, String>> maps) {
        Map<String, String> result = new HashMap<>();
        for (Map<String, String> map : maps) {
            if (map != null) {
                map.entrySet().stream().filter(entry -> !result.containsKey(entry.getKey()))
                        .forEach(entry -> result.put(entry.getKey(), entry.getValue()));
            }
        }
        return result;
    }

    @Override
    @Transactional
    public void delete(ResourceDeleteParams params) {
        String[] resources = params.getResources();
        if (resources == null || resources.length == 0) {
            return;
        }
        List<ProtectedResource> records = query(true, 0, resources.length,
                Collections.singletonMap("uuid", Arrays.asList(resources))).getRecords();
        List<ResourceDeleteContext> resourceDeleteContextList = new ArrayList<>();
        for (ProtectedResource record : records) {
            ResourceProvider resourceProvider = providerManager.findProvider(ResourceProvider.class, record, null);
            if (resourceProvider != null) {
                resourceDeleteContextList.add(resourceProvider.preHandleDelete(record));
            }
        }
        boolean isForce = params.isForce();
        String[] needDeleteResourceIds = handleDeleteResourceContext(params, resourceDeleteContextList, resources);
        boolean shouldDeleteRegister = params.isShouldDeleteRegister();
        ResourceDeleteParams afterHandleDeleteParams = new ResourceDeleteParams(isForce, shouldDeleteRegister,
                needDeleteResourceIds);
        List<String> deletedResourceUuids = repository.delete(afterHandleDeleteParams);
        // 删除资源标签
        labelService.deleteByResourceObjectIdsAndLabelIds(deletedResourceUuids.stream()
            .distinct()
            .collect(Collectors.toList()), StringUtils.EMPTY);
        for (String deletedResourceUuid : new HashSet<>(deletedResourceUuids)) {
            log.info("resource({}) deleted", deletedResourceUuid);
            resourceSetApi.deleteResourceSetRelation(deletedResourceUuid, ResourceSetTypeEnum.RESOURCE);
            messageTemplate.send("resource.deleted", new JSONObject().set("resource_id", deletedResourceUuid));
        }
    }

    // 返回要删除的资源ID
    private String[] handleDeleteResourceContext(ResourceDeleteParams params,
            List<ResourceDeleteContext> resourceDeleteContextList, final String[] resources) {
        Set<String> needCheckDependedOnIds = new HashSet<>();
        Set<String> deleteIdsFromContext = new HashSet<>();
        resourceDeleteContextList.stream().filter(Objects::nonNull)
                .filter(e -> !VerifyUtil.isEmpty(e.getResourceDeleteDependencyList()))
                .flatMap(e -> e.getResourceDeleteDependencyList().stream()).forEach(resourceDeleteDependency -> {
                    if (resourceDeleteDependency.isShouldCheckIfBeDependency()) {
                        needCheckDependedOnIds.addAll(resourceDeleteDependency.getDeleteIds());
                    }
                    deleteIdsFromContext.addAll(resourceDeleteDependency.getDeleteIds());
                });
        // 未从context返回的deleteIds也要参与check depended on
        needCheckDependedOnIds.addAll(
                Arrays.stream(resources).filter(e -> !deleteIdsFromContext.contains(e)).collect(Collectors.toList()));
        if (!params.isForce() && !params.isShouldIgnoreDependency() && needCheckDependedOnIds.size() > 0) {
            // 校验depended on
            checkIfBeDependedOn(needCheckDependedOnIds.toArray(new String[0]));
        }

        // 删除资源加上本身传入的
        deleteIdsFromContext.addAll(Arrays.asList(resources));
        return deleteIdsFromContext.toArray(new String[0]);
    }

    private void checkIfBeDependedOn(final String[] resources) {
        List<ProtectedResourceExtendInfoPo> resourceExtendInfos = resourceExtendInfoMapper
            .selectList(new QueryWrapper<ProtectedResourceExtendInfoPo>().in("RESOURCE_ID", Arrays.asList(resources))
                .likeRight("KEY", ResourceConstants.CITATION + ResourceConstants.CITATION_SEPERATOR));
        if (!VerifyUtil.isEmpty(resourceExtendInfos)) {
            ProtectedResourceExtendInfoPo resourceExtendInfoPo = resourceExtendInfos.get(0);
            // 如果还有关联资源 则打印日志并报错 取第一个资源uuid和type作为提示
            String dependedResourceId = getUuidByCitation(resourceExtendInfoPo);
            Optional<ProtectedResource> resource =
                resourceService.getBasicResourceById(resourceExtendInfoPo.getValue());
            if (resource.isPresent()) {
                log.error("Resource still have dependency, fail to delete, resource uuid:{}, total dependency size:{}",
                    dependedResourceId, resourceExtendInfos.size());
                throw new LegoCheckedException(CommonErrorCode.RESOURCE_BE_DEPENDED_BY_OTHERS,
                    new String[] {dependedResourceId, resource.get().getType()},
                    "The resource(ID: " + String.join(",", resources) + ") is depended by other resources.");
            }
            log.error(
                "Unknown Resource still have dependency, fail to delete, resource uuid:{}, total dependency size:{}",
                dependedResourceId, resourceExtendInfos.size());
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_BE_DEPENDED_BY_OTHERS,
                new String[] {dependedResourceId, "Unknown"},
                "The resource(ID: " + String.join(",", resources) + ") is depended by other resources.");
        }
    }

    /**
     * 从引用的资源取后缀 即从形如$citations_XXX_UUID 中取uuid
     *
     * @param resourceExtendInfoPo 资源表中的引用资源
     * @return uuid
     */
    private String getUuidByCitation(ProtectedResourceExtendInfoPo resourceExtendInfoPo) {
        // 从 resourceExtendInfoPo 中获取值
        String value = resourceExtendInfoPo.getValue();

        // 检查值是否为空
        if (VerifyUtil.isEmpty(value)) {
            log.error("Fail to find string from resource extend Info! uuid:{}", resourceExtendInfoPo.getUuid());
            return StringUtils.EMPTY;
        }

        // 以 "_" 进行分割
        String[] parts = value.split("_");

        // 检查分割后的数组是否符合预期
        if (parts.length < 1) {
            log.error("Fail to find string from resource extend Info! uuid:{}", resourceExtendInfoPo.getUuid());
            return StringUtils.EMPTY;
        }

        // 返回最后一个部分，即 UUID
        return parts[parts.length - 1];
    }

    /**
     * page query for protected resource
     *
     * @param shouldDecrypt decrypt flag
     * @param page page
     * @param size size
     * @param conditions page query conditions
     * @param orders orders
     * @return page data
     */
    @Override
    public PageListResponse<ProtectedResource> query(boolean shouldDecrypt, int page, int size,
            Map<String, Object> conditions, String... orders) {
        ResourceQueryParams context = new ResourceQueryParams();
        context.setPage(page);
        context.setSize(size);
        context.setConditions(conditions);
        context.setOrders(orders);
        context.setShouldDecrypt(shouldDecrypt);
        return query(context);
    }

    /**
     * page query for protected resource without loadEnvironment
     *
     * @param shouldDecrypt decrypt flag
     * @param page page
     * @param size size
     * @param conditions page query conditions
     * @param orders orders
     * @return page data
     */
    @Override
    public PageListResponse<ProtectedResource> basicQuery(boolean shouldDecrypt, int page, int size,
            Map<String, Object> conditions, String... orders) {
        ResourceQueryParams context = new ResourceQueryParams();
        context.setPage(page);
        context.setSize(size);
        context.setConditions(conditions);
        context.setOrders(orders);
        context.setShouldDecrypt(shouldDecrypt);
        context.setShouldLoadEnvironment(false);
        return query(context);
    }

    @Override
    public PageListResponse<ProtectedResource> query(ResourceQueryParams context) {
        int page = context.getPage();
        int size = context.getSize();
        Map<String, Object> conditions = context.getConditions();
        String[] orders = context.getOrders();
        boolean shouldDecrypt = context.isShouldDecrypt();
        ResourceRepositoryQueryParams params = new ResourceRepositoryQueryParams(context.isShouldIgnoreOwner(), page,
                size, conditions, orders);
        params.setDesesitization(context.isDesesitization());
        BasePage<ProtectedResource> data = repository.query(params).map(ProtectedResourcePo::toProtectedResource);
        List<ProtectedResource> resources = data.getItems();
        if (shouldDecrypt) {
            resources.forEach(this::decrypt);
        }
        if (context.isShouldLoadEnvironment()) {
            loadEnvironment(shouldDecrypt, 0, resources.size(), resources);
        }
        if (context.isShouldQueryDependency()) {
            resources.forEach(this::supplyDependency);
        }
        return new PageListResponse<>((int) data.getTotal(), data.getItems());
    }

    /**
     * page query for protected resource
     *
     * @param param 查询参数
     * @return page data
     */
    @Override
    public PageListResponse<ProtectedResourceGroupResult> groupQueryByExtendInfo(VstoreResourceQueryParam param) {
        if (param.getSize() <= 0) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "size cannot be less than or equal to 0.");
        }
        BasePage<ProtectedResourceGroupResult> data = repository.groupQuery(param);
        return new PageListResponse<>((int) data.getTotal(), data.getItems());
    }

    private void loadEnvironment(boolean shouldDecrypt, int page, int size, List<ProtectedResource> resources) {
        Set<String> environmentUuidList = resources.stream().filter(resource -> resource.getUuid() != null)
                .filter(resource -> !Objects.equals(resource.getRootUuid(), resource.getUuid()))
                .map(ProtectedResource::getRootUuid).filter(Objects::nonNull).collect(Collectors.toSet());
        if (environmentUuidList.isEmpty()) {
            return;
        }
        List<ProtectedEnvironment> environments = query(shouldDecrypt, page, environmentUuidList.size(),
                Collections.singletonMap("uuid", environmentUuidList)).getRecords().stream()
                        .filter(resource -> resource instanceof ProtectedEnvironment)
                        .map(ProtectedEnvironment.class::cast).collect(Collectors.toList());
        for (ProtectedResource resource : resources) {
            ProtectedEnvironment rootResource = environments.stream()
                    .filter(environment -> Objects.equals(environment.getUuid(), resource.getRootUuid())).findFirst()
                    .orElse(null);
            resource.setEnvironment(rootResource);
        }
    }

    /**
     * 查询资源
     * 适用场景：除了查询资源的基本属性外，还需要查询dependency依赖信息
     *
     * @param shouldDecrypt decrypt
     * @param resourceId 资源ID
     * @return 返回受保护资源
     */
    @Override
    public Optional<ProtectedResource> getResourceById(boolean shouldDecrypt, String resourceId) {
        if (resourceId == null) {
            return Optional.empty();
        }
        // 查询指定id资源
        Optional<ProtectedResource> resourceOptional = query(shouldDecrypt, 0, 1,
                Collections.singletonMap("uuid", resourceId)).getRecords().stream().findFirst();
        // 填充资源dependencies熟悉
        resourceOptional.ifPresent(this::supplyDependency);
        return resourceOptional;
    }

    @Override
    public Optional<ProtectedResource> getBasicResourceById(boolean shouldDecrypt, boolean shouldLoadEnvironment,
            String resourceId) {
        if (StringUtils.isBlank(resourceId)) {
            return Optional.empty();
        }
        if (shouldLoadEnvironment) {
            return query(shouldDecrypt, 0, 1, Collections.singletonMap("uuid", resourceId)).getRecords().stream()
                    .findFirst();
        } else {
            return basicQuery(shouldDecrypt, 0, 1, Collections.singletonMap("uuid", resourceId)).getRecords().stream()
                    .findFirst();
        }
    }

    /**
     * 补充资源的dependency信息
     * 使用场景：交给应用去决定是否执行补齐资源的dependency信息，该过程相对耗时
     *
     * @param resource 待补充dependency信息的资源
     * @return 返回受保护资源
     */
    @Override
    public Optional<ProtectedResource> queryDependency(ProtectedResource resource) {
        if (resource == null || StringUtils.isBlank(resource.getUuid())) {
            return Optional.empty();
        }
        this.supplyDependency(resource);
        return Optional.of(resource);
    }

    @Override
    public boolean checkHostIfBeTrustedByEndpoint(String endpoint, boolean isCheckWhenFalse) {
        if (!isHostTrustSystemSwitchOn()) {
            return true;
        }
        if (VerifyUtil.isEmpty(endpoint)) {
            return false;
        }
        boolean isTrusted;
        ProtectedEnvironment[] matchHost = new ProtectedEnvironment[1];
        if (isHyperDetect()) {
            log.debug("query endpoint trust in hyper detect. endpoint: {}", endpoint);
            // 防勒索
            isTrusted = checkHostIfBeTrustedHyperDetect(endpoint, matchHost);
        } else {
            isTrusted = checkHostIfBeTrustedCommon(endpoint, matchHost);
        }
        if (isCheckWhenFalse && !isTrusted) {
            String[] parameters = new String[2];
            parameters[0] = matchHost[0].getName();
            parameters[1] = matchHost[0].getEndpoint();
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_NOT_TRUST, parameters, "host do not be trusted.");
        }
        return isTrusted;
    }

    private boolean isHyperDetect() {
        return deployTypeService.isHyperDetectDeployType();
    }

    private boolean checkHostIfBeTrustedCommon(String endpoint, ProtectedEnvironment[] matchHost) {
        ProtectedEnvironment matchedProtectedResource = doQueryHostByEndpoint(endpoint);
        matchHost[0] = matchedProtectedResource;
        return checkResourceTrust(matchedProtectedResource);
    }

    private boolean checkHostIfBeTrustedHyperDetect(String endpoint, ProtectedEnvironment[] matchHost) {
        int page = 0;
        int size = 100;

        List<ProtectedEnvironment> records;
        do {
            ResourceQueryParams params = new ResourceQueryParams();
            params.setPage(page);
            params.setSize(size);
            params.setShouldIgnoreOwner(true);
            params.setShouldLoadEnvironment(false);
            Map<String, Object> conditions = new HashMap<>();
            conditions.put("type", ResourceTypeEnum.HOST.getType());
            params.setConditions(conditions);
            records = query(params).getRecords().stream().filter(e -> e instanceof ProtectedEnvironment)
                    .map(e -> (ProtectedEnvironment) e).collect(Collectors.toList());
            for (ProtectedEnvironment record : records) {
                String agentList = record.getExtendInfo().get(ResourceConstants.AGENT_IP_LIST);
                if (!matchEndpointInAgentList(endpoint, agentList)) {
                    continue;
                }
                matchHost[0] = record;
                return checkResourceTrust(record);
            }
            page++;
        } while (records.size() == size);
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "endpoint not found: " + endpoint);
    }

    private boolean isHostTrustSystemSwitchOn() {
        SystemSwitchDto systemSwitchDto = systemSwitchInternalService.queryByName(SwitchNameEnum.HOST_TRUST);
        return Objects.equals(systemSwitchDto.getStatus(), SwitchStatusEnum.ON);
    }

    private boolean matchEndpointInAgentList(String endpoint, String agentList) {
        String[] agentIps = resolveAgentList(agentList);
        for (String agentIp : agentIps) {
            if (Objects.equals(agentIp, endpoint)) {
                return true;
            }
        }
        return false;
    }

    private String[] resolveAgentList(String agentList) {
        if (VerifyUtil.isEmpty(agentList)) {
            return new String[0];
        }
        return agentList.split(",");
    }

    @Override
    public void checkHostIfBeTrusted(List<ProtectedEnvironment> records) {
        if (!isHostTrustSystemSwitchOn()) {
            return;
        }
        if (VerifyUtil.isEmpty(records)) {
            return;
        }
        // 检查类型为Host的资源
        List<ProtectedEnvironment> resourceList = records.stream()
                .filter(resource -> ResourceTypeEnum.HOST.getType().equalsIgnoreCase(resource.getType()))
                .collect(Collectors.toList());
        for (ProtectedEnvironment record : resourceList) {
            if (!checkResourceTrust(record)) {
                String[] parameters = new String[2];
                parameters[0] = record.getName();
                parameters[1] = record.getEndpoint();
                throw new LegoCheckedException(CommonErrorCode.RESOURCE_NOT_TRUST, parameters,
                        "host do not be trusted.");
            }
        }
    }

    @Override
    public void desensitize(ProtectedResource resource) {
        Queue<ProtectedResource> queue = new LinkedList<>();
        queue.offer(resource);
        while (queue.size() > 0) {
            ProtectedResource pollResource = queue.poll();
            if (pollResource == null) {
                continue;
            }
            protectedResourceMonitorService.invoke("desensitize", pollResource, ProtectedResourceEvent::getResource);
            if (VerifyUtil.isEmpty(pollResource.getDependencies())) {
                continue;
            }
            pollResource.getDependencies().values().stream().filter(Objects::nonNull).flatMap(Collection::stream)
                    .forEach(queue::offer);
        }
    }

    @Override
    public void appendGroupInfo(ProtectedResource resource) {
        String uuid = resource.getUuid();
        List<ResourceGroupMemberPo> resourceGroupMemberPos = resourceGroupMemberMapper.selectByResourceId(uuid);

        if (VerifyUtil.isEmpty(resourceGroupMemberPos)) {
            resource.setInGroup(false);
        } else {
            resource.setInGroup(true);
            ResourceGroupPo resourceGroupPo =
                    resourceGroupMapper.selectById(resourceGroupMemberPos.get(0).getResourceGroupId());
            if (resourceGroupPo != null) {
                resource.setResourceGroupName(resourceGroupPo.getName());
                resource.setResourceGroupId(resourceGroupPo.getUuid());
            }
        }
    }

    private boolean checkResourceTrust(ProtectedResource resource) {
        Map<String, String> extendInfo = resource.getExtendInfo();
        if (extendInfo == null) {
            return false;
        }
        if (isInternalAgent(resource)) {
            return true;
        }
        String trustValue = extendInfo.get(ResourceExtendInfoKeyConstants.TRUSTWORTHINESS);
        return "true".equalsIgnoreCase(trustValue);
    }

    private boolean isInternalAgent(ProtectedResource resource) {
        Map<String, String> extendInfo = resource.getExtendInfo();
        if (extendInfo == null) {
            return false;
        }
        String scenarioValue = extendInfo.get(ResourceExtendInfoKeyConstants.EXT_INFO_SCENARIO);
        return scenarioValue != null && scenarioValue.equals(AgentTypeEnum.INTERNAL_AGENT.getValue());
    }

    private void supplyDependency(ProtectedResource resource) {
        log.debug("supply dependency start, resource id: {}", resource.getUuid());
        // 是否插件执行
        ResourceProvider resourceProvider = providerManager.findProvider(ResourceProvider.class, resource, null);
        if (resourceProvider != null && resourceProvider.supplyDependency(resource)) {
            return;
        }
        // 查出所有相关资源(自己和所有子资源以及自己和子资源的所有依赖项)Map
        // resourceMap的key为uuid value为资源对象
        Map<String, ProtectedResource> resourceMap =
            queryAllRelatedAndDependencyResources(true, resource.getUuid(), resource);
        // 再将自己put进去 似乎没必要 因为上边查询中一定会查出自己
        resourceMap.put(resource.getUuid(), resource);
        log.debug("All related and dependency resources size: {}, resource id: {}.",
            resourceMap.size(), resource.getUuid());
        // 填充资源的依赖
        doSupplyDependency(resourceMap);
        log.debug("default supply dependency end, resource id: {}", resource.getUuid());
    }

    private void doSupplyDependency(Map<String, ProtectedResource> resourceMap) {
        // 遍历所有的相关资源 查看该资源是否存在extendInfo(res_extend_info表中有uuid为此资源的记录)
        for (Map.Entry<String, ProtectedResource> entry : resourceMap.entrySet()) {
            Map<String, String> extendInfo = entry.getValue().getExtendInfo();
            if (extendInfo == null) {
                continue;
            }
            for (Map.Entry<String, String> ex : extendInfo.entrySet()) {
                // 如果res_extend_info表中不存在$citations开头的key 说明该资源没有被其它资源依赖 下一位
                // 依赖key示例 $citations_agents_3675acde49de446088d978b3a5d4e97a
                if (!ex.getKey().startsWith(ResourceConstants.CITATION)) {
                    continue;
                }
                // 获取依赖类型（示例中为agents）
                Optional<String> dependencyKey = getDependencyKey(ex.getKey());
                if (!dependencyKey.isPresent()) {
                    continue;
                }
                // 如果依赖该资源的资源存在于相关资源map中 则填充相关资源的dependencies属性
                ProtectedResource dResource = resourceMap.get(ex.getValue());
                if (dResource == null) {
                    continue;
                }
                Map<String, List<ProtectedResource>> dependencies = dResource.getDependencies();
                if (dependencies == null) {
                    dependencies = new HashMap<>();
                    dResource.setDependencies(dependencies);
                }
                List<ProtectedResource> protectedResources = dependencies.computeIfAbsent(dependencyKey.get(),
                        e -> new ArrayList<>());
                protectedResources.add(entry.getValue());
            }
        }
    }

    private Optional<String> getDependencyKey(String citationFullKey) {
        String[] split = citationFullKey.split(ResourceConstants.CITATION_SEPERATOR);
        if (split.length < 2) {
            return Optional.empty();
        }
        return Optional.of(split[1]);
    }

    private void decrypt(ProtectedResource resource) {
        protectedResourceMonitorService.invoke("decrypt", resource, ProtectedResourceEvent::getResource);
    }

    /**
     * query related resource uuid
     *
     * @param parentUuids parent resource uuids
     * @param excludeResourceUuids excluded resource uuids
     * @return related resource uuids
     */
    public Set<String> queryRelatedResourceUuids(List<String> parentUuids, String... excludeResourceUuids) {
        return repository.queryRelatedResourceUuids(parentUuids, excludeResourceUuids);
    }

    @Override
    public List<ProtectedResource> queryDependencyResources(boolean shouldDecrypt, final String key,
            final List<String> uuids) {
        List<ProtectedResource> resourceList = repository.queryDependencyResources(key, uuids).stream()
                .map(ProtectedResourcePo::toProtectedResource).collect(Collectors.toList());
        if (shouldDecrypt) {
            resourceList.forEach(this::decrypt);
        }
        return resourceList;
    }

    @Override
    public List<ProtectedResource> queryResourcesByUserId(String userId) {
        return repository.queryResourcesByUserId(userId).stream().map(ProtectedResourcePo::toProtectedResource)
                .peek(this::decrypt).collect(Collectors.toList());
    }

    @Override
    public PageListResponse<ProtectedResource> queryAgentResourceList(Map<String, Object> map) {
        TokenBo.UserBo user = sessionService.getCurrentUser();
        if (user != null && !DefaultRoleHelper.isAdminOrAudit(user.getId())) {
            String domainId = user.getDomainId();
            map.put("domainId", domainId);
        }

        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        if (map.get("pluginType") != null) {
            List<String> filteredByPluginTypeAgentIds =
                queryAgentIdsByPluginType(String.valueOf(map.get("pluginType")));
            // 无对应插件类型主机，提前退出
            if (filteredByPluginTypeAgentIds.size() == 0) {
                response.setTotalCount(0);
                response.setRecords(new ArrayList<>());
                return response;
            }
            map.put("filteredByPluginTypeIds", filteredByPluginTypeAgentIds);
        }
        if (!VerifyUtil.isEmpty(map.get("labelName")) && user != null && !VerifyUtil.isEmpty(user.getName())) {
            map.put("userName", user.getName());
        }
        int count = repository.queryAgentResourceCount(map);
        int startIndex = (int) map.get(PAGE_SIZE) * (int) map.get(PAGE_NO);
        int endIndex = startIndex + (int) map.get(PAGE_SIZE);
        if (endIndex > count) {
            endIndex = count;
        }
        response.setTotalCount(count);
        if (startIndex > endIndex) {
            response.setRecords(new ArrayList<>());
            return response;
        }
        List<ProtectedResource> protectedResourceList = repository.queryAgentResourceList(map)
            .stream()
            .map(ProtectedResourcePo::toProtectedResource)
            .collect(Collectors.toList());
        if (map.containsKey(CPU_RATE) || map.containsKey(MEM_RATE)) {
            List<ProtectedResource> mergedProtectedResourceList = getWithRateAgentResources(protectedResourceList);
            response.setRecords(mergedProtectedResourceList.subList(startIndex, endIndex));
            return response;
        }
        response.setRecords(protectedResourceList.subList(startIndex, endIndex));
        return response;
    }

    private List<ProtectedResource> getWithRateAgentResources(List<ProtectedResource> protectedResourceList) {
        List<ProtectedResource> mergedProtectedResourceList = new ArrayList<>();
        List<ProtectedResource> onlineAndExtendResourceList = protectedResourceList.stream()
            .filter(protectedResource -> linkStatusIsOnlineAndHasExtend(protectedResource))
            .collect(Collectors.toList());
        List<ProtectedResource> onlineAndExtendNullResourceList = protectedResourceList.stream()
            .filter(protectedResource -> linkStatusIsOnlineAndExtendNull(protectedResource))
            .collect(Collectors.toList());
        List<ProtectedResource> notOnlineResourceList = protectedResourceList.stream()
            .filter(protectedResource -> linkStatusNotOnline(protectedResource))
            .collect(Collectors.toList());
        mergedProtectedResourceList.addAll(onlineAndExtendResourceList);
        mergedProtectedResourceList.addAll(onlineAndExtendNullResourceList);
        mergedProtectedResourceList.addAll(notOnlineResourceList);
        return mergedProtectedResourceList;
    }

    private List<String> queryAgentIdsByPluginType(String pluginType) {
        Set<String> agentIds = new HashSet<>();
        Map<String, Object> filter = new HashMap<>();
        filter.put("type", "Plugin");
        filter.put("subType", pluginType);
        ResourceQueryParams queryParams = new ResourceQueryParams();
        queryParams.setPage(0);
        queryParams.setSize(QUERY_PAGE_SIZE);
        queryParams.setConditions(filter);
        queryParams.setShouldIgnoreOwner(true);
        List<String> queryResult = queryByFilter(queryParams);
        while (!queryResult.isEmpty()) {
            agentIds.addAll(queryResult);
            queryParams.setPage(queryParams.getPage() + 1);
            queryResult = queryByFilter(queryParams);
        }
        return new ArrayList<>(agentIds);
    }

    private List<String> queryByFilter(ResourceQueryParams queryParams) {
        return query(queryParams).getRecords()
            .stream()
            .map(ResourceBase::getParentUuid)
            .filter(Objects::nonNull)
            .distinct()
            .collect(Collectors.toList());
    }

    private boolean linkStatusIsOnlineAndHasExtend(ProtectedResource protectedResource) {
        ProtectedEnvironment protectedEnvironment = null;
        if (protectedResource instanceof ProtectedEnvironment) {
            protectedEnvironment = (ProtectedEnvironment) protectedResource;
        }
        return String.valueOf(LinkStatusEnum.ONLINE.getStatus()).equals(protectedEnvironment.getLinkStatus())
            && Objects.nonNull(protectedResource.getProtectedAgentExtend());
    }

    private boolean linkStatusIsOnlineAndExtendNull(ProtectedResource protectedResource) {
        ProtectedEnvironment protectedEnvironment = null;
        if (protectedResource instanceof ProtectedEnvironment) {
            protectedEnvironment = (ProtectedEnvironment) protectedResource;
        }
        return String.valueOf(LinkStatusEnum.ONLINE.getStatus()).equals(protectedEnvironment.getLinkStatus())
                && Objects.isNull(protectedResource.getProtectedAgentExtend());
    }

    private boolean linkStatusNotOnline(ProtectedResource protectedResource) {
        ProtectedEnvironment protectedEnvironment = null;
        if (protectedResource instanceof ProtectedEnvironment) {
            protectedEnvironment = (ProtectedEnvironment) protectedResource;
        }
        return !String.valueOf(LinkStatusEnum.ONLINE.getStatus()).equals(protectedEnvironment.getLinkStatus());
    }

    private Map<String, ProtectedResource> queryAllRelatedAndDependencyResources(boolean shouldDecrypt, String uuid,
        ProtectedResource protectedResource) {
        // 查询所有相关资源uuid（包括子资源和res_extend_info表中的依赖资源）
        // sql内容为先递归查询出所有的子资源[uuids] 再查询res_extend_info的value为[uuids]的资源
        // 即查询出自己和所有的子资源及其依赖项(dependencies)的uuid 此处子资源可能有相同的依赖项 会查出多个 因此通过set去重
        Set<String> uuids = protectedResourceMapper.queryResourceDependencyRelation(Collections.singletonList(uuid))
            .stream()
            .map(ResourceDependencyRelation::getUuid)
            .collect(Collectors.toSet());

        log.debug("Base on resource(ID: {}), found these dependency resources in database, resource size: {}.", uuid,
            uuids.size());
        // 根据相关资源uuid列表查询resource对象 此处不会再去查询依赖项
        List<ProtectedResource> protectedResources =
            getProtectedResourcesByIds(shouldDecrypt, uuids, protectedResource);
        return protectedResources.stream().collect(Collectors.toMap(ProtectedResource::getUuid, Function.identity()));
    }

    private List<ProtectedResource> getProtectedResourcesByIds(boolean shouldDecrypt, Set<String> uuids,
        ProtectedResource protectedResource) {
        // pageSize太多，分页查询, 每次最大500
        Set<String> subUuids = new HashSet<>();
        List<ProtectedResource> protectedResources = new ArrayList<>();
        for (String id : uuids) {
            subUuids.add(id);
            if (subUuids.size() >= MAX_PAGE_NUM) {
                // 如果是HCS OP服务化环境中的6款应用或NAS共享或线下HCS环境 则查询时不校验资源所属用户
                if (shouldIgnoreUser(protectedResource)) {
                    protectedResources.addAll(
                        queryIgnoreOwner(shouldDecrypt, 0, subUuids.size(), Collections.singletonMap("uuid", subUuids))
                            .getRecords());
                } else {
                    protectedResources
                        .addAll(query(shouldDecrypt, 0, subUuids.size(), Collections.singletonMap("uuid", subUuids))
                            .getRecords());
                }
                subUuids.clear();
            }
        }
        if (subUuids.size() > 0) {
            if (shouldIgnoreUser(protectedResource)) {
                protectedResources.addAll(
                    queryIgnoreOwner(shouldDecrypt, 0, subUuids.size(), Collections.singletonMap("uuid", subUuids))
                        .getRecords());
            } else {
                protectedResources.addAll(
                    query(shouldDecrypt, 0, subUuids.size(), Collections.singletonMap("uuid", subUuids)).getRecords());
            }
        }
        log.debug("get related resources size: {}", uuids.size());
        return protectedResources;
    }

    private boolean shouldIgnoreUser(ProtectedResource protectedResource) {
        if (protectedResource.getSubType().equals(ResourceSubTypeEnum.NAS_SHARE.getType())
            || protectedResource.getSubType().equals(ResourceSubTypeEnum.HCS_CONTAINER.getType())) {
            return true;
        }
        if (OpServiceUtil.isHcsService() && HCS_OP_LIST.contains(protectedResource.getSubType())) {
            return true;
        }
        return false;
    }

    @Override
    public boolean checkEnvScanTaskIsRunning(String resId) {
        QueryJobRequest request = new QueryJobRequest();
        request.setSourceId(resId);
        request.setTypes(
                Collections.singletonList(ResourceConstant.JOB_TYPE_PREFIX + ResourceConstant.MANUAL_SCAN_RESOURCE));
        request.setStatusList(RUNNING_AND_PENDING_STATUS_LIST.stream().map(JobStatusEnum::name)
                .collect(Collectors.toList()));
        PagingParamRequest pagingParamRequest = new PagingParamRequest();
        SortingParamRequest sortingParamRequest = new SortingParamRequest();
        int size = jobService.queryJobs(request, pagingParamRequest, sortingParamRequest).getRecords().size();
        return size > 0;
    }

    @AllArgsConstructor
    private static class ResourceNode {
        static int add = 1;

        static int update = 2;

        // 1.add 2.update
        int type;

        ProtectedResource resource;
    }

    /**
     * 根据 root uuid 查询所有资源的uuid
     *
     * @param path path
     * @param rootUuid rootuuid
     * @return uuid set
     */
    @Override
    public List<ProtectedResource> queryAllResourceIdsByPathAndRootUuid(String path, String rootUuid) {
        return repository.queryAllResourceIdsByPathAndRootUuid(path, rootUuid);
    }

    /**
     * 填充依赖数据
     *
     * @param resource 待填充依赖的资源
     */
    @Override
    public void setResourceDependency(ProtectedResource resource) {
        Set<String> resIds = getRelatedResourceIds(resource);
        List<ProtectedResource> resources = getProtectedResourcesByIds(true, resIds, resource);
        Map<String, ProtectedResource> resourceMap = resources.stream()
                .collect(Collectors.toMap(ProtectedResource::getUuid, Function.identity()));
        resourceMap.put(resource.getUuid(), resource);
        // 填充资源的依赖
        doSupplyDependency(resourceMap);
        log.info("special supply dependency end, resource id: {}", resource.getUuid());
    }

    @Override
    public ProtectedEnvironment queryHostByEndpoint(String endpoint) {
        return doQueryHostByEndpoint(endpoint);
    }

    private ProtectedEnvironment doQueryHostByEndpoint(String endpoint) {
        ResourceQueryParams params = new ResourceQueryParams();
        params.setShouldIgnoreOwner(true);
        params.setShouldLoadEnvironment(false);
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("endpoint", endpoint);
        conditions.put("type", ResourceTypeEnum.HOST.getType());
        params.setConditions(conditions);
        List<ProtectedEnvironment> records = query(params).getRecords().stream()
                .filter(e -> e instanceof ProtectedEnvironment).map(e -> (ProtectedEnvironment) e)
                .collect(Collectors.toList());
        if (records.isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "endpoint not found: " + endpoint);
        }
        return records.get(0);
    }

    /**
     * 通过agent id集合，获取其存在扩展表中的LanFree配置项
     *
     * @param subType 资源类型
     * @param resourceIds agent id集合
     * @return <agentId, lanFree配置项("true"-已配置，“false"-未配置)>
     */
    @Override
    public Map<String, String> getLanFreeConfig(String subType, List<String> resourceIds) {
        if (VerifyUtil.isEmpty(resourceIds) || !supportLanFree(subType)) {
            return new HashMap<>();
        }
        QueryWrapper<ProtectedResourceExtendInfoPo> wrapper = new QueryWrapper<ProtectedResourceExtendInfoPo>()
                .eq(ResourceConstants.KEY, ResourceConstants.IS_ADD_LAN_FREE)
                .in(ResourceConstants.RESOURCE_ID, resourceIds);
        return resourceExtendInfoMapper.selectList(wrapper).stream().filter(Objects::nonNull)
                .collect(Collectors.toMap(ProtectedResourceExtendInfoPo::getResourceId,
                        extendInfoPo -> String.valueOf(extendInfoPo.getValue().equals("1"))));
    }

    @Override
    @Transactional
    public void modifyNextBackup(NextBackupModifyReq req, boolean isStrict) {
        if (VerifyUtil.isEmpty(req.getResourceIds())) {
            return;
        }
        req.getResourceIds().forEach(resourceId -> this.modifySingleResourceNextBackup(req, resourceId, isStrict));
    }

    private void modifySingleResourceNextBackup(NextBackupModifyReq req, String resourceId, boolean isStrict) {
        Map<String, String> extendInfo = new HashMap<>();
        String nextBackupType = req.getNextBackupParams().getNextBackupType();
        String nextBackupChangeCause = req.getNextBackupParams().getNextBackupChangeCause();
        extendInfo.put(ResourceExtendInfoConstants.NEXT_BACKUP_TYPE_EXT_KEY, nextBackupType);
        extendInfo.put(ResourceExtendInfoConstants.NEXT_BACKUP_CHANGE_CAUSE_EXT_KEY, nextBackupChangeCause);
        log.info("ModifyNextBackup resource id: {}, next_backup_type: {}, next_backup_change_cause: {}.", resourceId,
                nextBackupType, nextBackupChangeCause);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(resourceId);
        protectedResource.setExtendInfo(extendInfo);
        try {
            this.updateSourceDirectly(Collections.singletonList(protectedResource));
        } catch (Exception e) {
            if (isStrict) {
                throw LegoCheckedException.cast(e);
            }
            log.warn("Resource not exist, modify next backup failed, resourceId: {}.", resourceId);
        }
    }

    @Override
    public void cleanNextBackup(String resourceId) {
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(ResourceExtendInfoConstants.NEXT_BACKUP_TYPE_EXT_KEY, StringUtils.EMPTY);
        extendInfo.put(ResourceExtendInfoConstants.NEXT_BACKUP_CHANGE_CAUSE_EXT_KEY, StringUtils.EMPTY);
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(resourceId);
        protectedResource.setExtendInfo(extendInfo);
        resources.add(protectedResource);
        this.updateSourceDirectly(resources);
    }

    @Override
    public NextBackupParams queryNextBackupTypeAndCause(String resourceId) {
        Optional<ProtectedResource> basicResource = this.getBasicResourceById(resourceId);
        Map<String, String> extendInfo = basicResource.map(ProtectedResource::getExtendInfo)
                .orElse(Collections.emptyMap());
        String nextBackupType = extendInfo.getOrDefault(ResourceExtendInfoConstants.NEXT_BACKUP_TYPE_EXT_KEY,
                StringUtils.EMPTY);
        String nextBackupChangeCause = extendInfo
                .getOrDefault(ResourceExtendInfoConstants.NEXT_BACKUP_CHANGE_CAUSE_EXT_KEY, StringUtils.EMPTY);
        return new NextBackupParams(nextBackupType, nextBackupChangeCause);
    }

    @Override
    public List<String> getRelationInLanFree(String subType, List<String> resourceIds) {
        if (VerifyUtil.isEmpty(resourceIds) || !supportLanFree(subType)) {
            return new ArrayList<>();
        }
        List<String> lanFreeAgentIds = filterIsLanFreeResourceIds(resourceIds);
        QueryWrapper<ProtectedResourceExtendInfoPo> wrapper = new QueryWrapper<ProtectedResourceExtendInfoPo>()
                .eq(ResourceConstants.KEY, ResourceConstants.CLUSTER_ESN)
                .in(ResourceConstants.RESOURCE_ID, lanFreeAgentIds);
        return resourceExtendInfoMapper.selectList(wrapper).stream().filter(Objects::nonNull)
                .map(ProtectedResourceExtendInfoPo::getValue).filter(StringUtils::isNotBlank)
                .collect(Collectors.toList());
    }

    @Override
    public List<ProtectedResource> getResourceByParentId(String resourceId) {
        if (resourceId == null) {
            return new ArrayList<>();
        }
        return query(false, 0, maxResourceNum,
            Collections.singletonMap("parentUuid", resourceId)).getRecords();
    }

    @Override
    public List<ProtectedResource> getResourceByParentIdIgnoreOwner(String resourceId) {
        if (resourceId == null) {
            return new ArrayList<>();
        }
        return repository.queryResourcesByParentUuid(resourceId);
    }

    @Override
    public Optional<String> getResourceIdByExKeyValue(String key, String value) {
        if (VerifyUtil.isEmpty(key) || VerifyUtil.isEmpty(value)) {
            return Optional.empty();
        }
        QueryWrapper<ProtectedResourceExtendInfoPo> wrapper = new QueryWrapper<ProtectedResourceExtendInfoPo>().eq(
            ResourceConstants.KEY, key).eq(ResourceConstants.VALUE, value);
        return resourceExtendInfoMapper.selectList(wrapper)
            .stream()
            .findFirst().map(ProtectedResourceExtendInfoPo::getResourceId);
    }

    @Override
    public List<String> getResourceIdsByExKeyValue(String key, String value) {
        if (VerifyUtil.isEmpty(key) || VerifyUtil.isEmpty(value)) {
            return new ArrayList<>();
        }
        QueryWrapper<ProtectedResourceExtendInfoPo> wrapper = new QueryWrapper<ProtectedResourceExtendInfoPo>().eq(
            ResourceConstants.KEY, key).eq(ResourceConstants.VALUE, value);
        return resourceExtendInfoMapper.selectList(wrapper)
            .stream()
            .map(protectedResourceExtendInfoPo -> protectedResourceExtendInfoPo.getResourceId())
            .distinct()
            .collect(Collectors.toList());
    }

    /**
     * 过滤出开启了lanFree的agent 每个agent都会去判断
     *
     * @param resourceIds 过滤前资源ids
     * @return 过滤后，开启了lanFree的资源ids
     */
    private List<String> filterIsLanFreeResourceIds(List<String> resourceIds) {
        QueryWrapper<ProtectedResourceExtendInfoPo> wrapper = new QueryWrapper<ProtectedResourceExtendInfoPo>()
                .eq(ResourceConstants.KEY, ResourceConstants.IS_ADD_LAN_FREE)
                .in(ResourceConstants.RESOURCE_ID, resourceIds);
        List<String> lanfreeAgentIds = resourceExtendInfoMapper.selectList(wrapper).stream().filter(Objects::nonNull)
                .filter(extendInfoPo -> IS_LAN_FREE.equals(extendInfoPo.getValue()))
                .map(ProtectedResourceExtendInfoPo::getResourceId).collect(Collectors.toList());
        if (lanfreeAgentIds.isEmpty()) {
            return new ArrayList<>();
        }
        return lanfreeAgentIds;
    }

    /**
     * 判断应用是否支持LanFree
     *
     * @param subType 资源subType
     * @return 是否支持LanFree，true支持，false不支持
     */
    private boolean supportLanFree(String subType) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(subType);
        ResourceProvider provider = providerManager.findProvider(ResourceProvider.class, protectedResource, null);
        return (provider == null || (provider != null && provider.getResourceFeature().isSupportedLanFree()));
    }

    private Set<String> getRelatedResourceIds(ProtectedResource resource) {
        QueryWrapper<ProtectedResourceExtendInfoPo> extendWrapper = new QueryWrapper<ProtectedResourceExtendInfoPo>();
        extendWrapper.eq("value", resource.getUuid());
        extendWrapper.like("key", ResourceConstants.CITATION + ResourceConstants.CITATION_SEPERATOR);
        List<ProtectedResourceExtendInfoPo> extendInfoPos = resourceExtendInfoMapper.selectList(extendWrapper);
        if (VerifyUtil.isEmpty(extendInfoPos)) {
            return new HashSet<>();
        }
        Set<String> resIds = extendInfoPos.stream().map(ProtectedResourceExtendInfoPo::getResourceId)
                .collect(Collectors.toSet());
        log.info("based on resId:{}, get dependency resource size:{}", resource.getUuid(), resIds.size());
        return resIds;
    }
    @Override
    public void updateUserId(String uuid, String userId, String authorizedUserName) {
        repository.updateUserId(uuid, userId, authorizedUserName);
    }

    /**
     * 查询资源
     * 适用场景：除了查询资源的基本属性外，还需要查询dependency依赖信息
     *
     * @param shouldDecrypt decrypt
     * @param resourceId 资源ID
     * @return 返回受保护资源
     */
    @Override
    public Optional<ProtectedResource> getResourceByIdIgnoreOwner(boolean shouldDecrypt, String resourceId) {
        if (resourceId == null) {
            return Optional.empty();
        }
        Optional<ProtectedResource> resourceOptional = queryIgnoreOwner(shouldDecrypt, 0, 1,
            Collections.singletonMap("uuid", resourceId)).getRecords().stream().findFirst();
        resourceOptional.ifPresent(this::supplyDependency);
        return resourceOptional;
    }

    /**
     * 更新插件资源资源用户信息为空
     *
     * @param parentUuid 主机资源id
     */
    @Override
    public void updatePluginResourceUserId(String parentUuid) {
        protectedResourceMapper.updatePluginResourceUserId(parentUuid);
    }

    /**
     * page query for protected resource
     *
     * @param shouldDecrypt decrypt flag
     * @param page page
     * @param size size
     * @param conditions page query conditions
     * @param orders orders
     * @return page data
     */
    public PageListResponse<ProtectedResource> queryIgnoreOwner(boolean shouldDecrypt, int page, int size,
        Map<String, Object> conditions, String... orders) {
        ResourceQueryParams context = new ResourceQueryParams();
        context.setPage(page);
        context.setSize(size);
        context.setShouldIgnoreOwner(true);
        context.setConditions(conditions);
        context.setOrders(orders);
        context.setShouldDecrypt(shouldDecrypt);
        return query(context);
    }

    /**
     * 查询资源是否支持恢复
     *
     * @param resourceId 资源ID
     * @return 当前资源是否支持恢复
     */
    @Override
    public String judgeResourceRestoreLevel(String resourceId) {
        String isAllowRestoreFlag = ResourceConstants.ALLOW_RESTORE;

        // 1、判断当前资源是否支持恢复
        log.info("Start to check cur resource restore limit");
        ProtectedResource resource = getResourceById(false, resourceId).orElseThrow(
            () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
        String isAllowRestore = resource.getExtendInfo().get(ResourceConstants.IS_ALLOW_RESTORE_KEY);
        if (notAllowRestoreBase(isAllowRestore, resource.getSubType())) {
            isAllowRestoreFlag = ResourceConstants.NOT_ALLOW_RESTORE;
        }

        // 部分类型资源不支持界面上设置是否可以恢复，默认支持恢复
        if (ResourceConstants.ALWAYS_ALLOW_RESTORE_RESOURCE.contains(resource.getSubType())) {
            log.info("Database skip check resource restore limit");
            isAllowRestoreFlag = ResourceConstants.ALLOW_RESTORE;
        }

        // 2、判断当前资源父资源是否支持恢复
        log.info("Start to check parent resource restore limit");
        ProtectedResource parentResource = resource;

        // 循环获取父资源，直到根资源
        isAllowRestoreFlag = judgeParentResourceAllowRestore(isAllowRestoreFlag, parentResource);

        // 3、判断当前资源子资源是否支持恢复
        log.info("Start to check child resource restore limit");
        Set<String> relatedResourceUuids = queryRelatedResourceUuids(resource.getUuid(), new String[] {});
        for (String relatedResourceUuid : relatedResourceUuids) {
            resource = getResourceById(false, relatedResourceUuid).orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));

            // 部分类型资源不支持界面上设置是否可以恢复，所以不参与校验
            if (ResourceConstants.ALWAYS_ALLOW_RESTORE_RESOURCE.contains(resource.getSubType())) {
                continue;
            }
            isAllowRestore = resource.getExtendInfo().get(ResourceConstants.IS_ALLOW_RESTORE_KEY);
            if (notAllowRestoreBase(isAllowRestore, resource.getSubType())) {
                isAllowRestoreFlag = ResourceConstants.NOT_ALLOW_RESTORE;
                log.info("child resource {} not allow restore, check end", relatedResourceUuid);
                break;
            }
        }
        return isAllowRestoreFlag;
    }

    private boolean notAllowRestoreBase(String isAllowRestoreBase, String resourceSubType) {
        boolean isAllowRestore = false;
        if (resourceSubType.equals(ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType())) {
            if (StringUtils.equals(isAllowRestoreBase, ResourceConstants.NOT_ALLOW_RESTORE)) {
                isAllowRestore = true;
            }
        } else {
            if (StringUtils.isBlank(isAllowRestoreBase) || StringUtils.equals(isAllowRestoreBase,
                    ResourceConstants.NOT_ALLOW_RESTORE)) {
                isAllowRestore = true;
            }
        }
        return isAllowRestore;
    }

    private String judgeParentResourceAllowRestore(String isAllowRestoreFlag, ProtectedResource resource) {
        String rootUuid = "";
        String uuid = resource.getUuid();
        String allowRestoreFlag = isAllowRestoreFlag;
        ProtectedResource parentResource = resource;
        do {
            String parentUuid = parentResource.getParentUuid();
            if (StringUtils.isBlank(parentUuid)) {
                log.info("Parent uuid is null, check end");
                break;
            }
            parentResource = getResourceById(false, parentUuid).orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));

            // 部分类型资源不支持界面上设置是否可以恢复，所以不参与校验
            if (ResourceConstants.ALWAYS_ALLOW_RESTORE_RESOURCE.contains(parentResource.getSubType())) {
                continue;
            }

            String isAllowRestore = parentResource.getExtendInfo().get(ResourceConstants.IS_ALLOW_RESTORE_KEY);
            if (notAllowRestoreBase(isAllowRestore, resource.getSubType())) {
                allowRestoreFlag = ResourceConstants.NOT_ALLOW_RESTORE;
                log.info("Parent resource {} not allow restore, check end", parentUuid);
                break;
            }
            rootUuid = parentResource.getRootUuid();
            uuid = parentResource.getUuid();
        } while (!StringUtils.equals(rootUuid, uuid));
        return allowRestoreFlag;
    }

    /**
     * 检查父资源是否支持恢复
     *
     * @param resourceId 资源ID
     */
    @Override
    public void verifyParentResourceRestoreLevel(String resourceId) {
        // 2、判断当前资源父资源是否支持恢复
        log.info("Start to check parent resource restore limit");
        ProtectedResource parentResource = getResourceById(false, resourceId).orElseThrow(
            () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));

        String rootUuid = parentResource.getRootUuid();
        String uuid = parentResource.getUuid();
        if (StringUtils.equals(rootUuid, uuid)) {
            log.info("Root resource no need to check parent resource restore limit");
            return;
        }

        // 循环获取父资源，直到根资源
        do {
            String parentUuid = parentResource.getParentUuid();
            if (StringUtils.isBlank(parentUuid)) {
                log.info("Parent uuid is null, check end");
                break;
            }
            parentResource = getResourceById(false, parentUuid).orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
            String isAllowRestore = parentResource.getExtendInfo().get(ResourceConstants.IS_ALLOW_RESTORE_KEY);

            // 部分类型资源不支持界面上设置是否可以恢复，所以不参与校验
            if (ResourceConstants.ALWAYS_ALLOW_RESTORE_RESOURCE.contains(parentResource.getSubType())) {
                continue;
            }

            // 当有父资源不支持恢复时，报错
            if (StringUtils.isBlank(isAllowRestore) || StringUtils.equals(isAllowRestore,
                ResourceConstants.NOT_ALLOW_RESTORE)) {
                log.info("Parent resource {} not allow restore, check end", parentUuid);
                throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "Parent Resource not allow restore");
            }
            rootUuid = parentResource.getRootUuid();
            uuid = parentResource.getUuid();
        } while (!StringUtils.equals(rootUuid, uuid));
    }


    /**
     * 根据用户id获取用户域下所有的资源id集合
     *
     * @param userId 用户id
     * @return 用户域下资源id集合
     */
    @Override
    public List<String> getDomainResourceIdListByUserId(String userId) {
        DomainInfoBo domainInfoBo = domainService.getDomainInfoByUserId(userId);
        return domainResourceSetService.getResourceIdList(domainInfoBo.getUuid(),
            ResourceSetTypeEnum.RESOURCE);
    }

    @Override
    public List<ProtectedResource> getResourceListByUuidList(List<String> resourceIds) {
        if (VerifyUtil.isEmpty(resourceIds)) {
            return new ArrayList<>();
        }
        LambdaQueryWrapper<ProtectedResourcePo> wrapper = new LambdaQueryWrapper<ProtectedResourcePo>()
            .in(ProtectedResourcePo::getUuid, resourceIds);
        return protectedResourceMapper.selectList(wrapper).stream()
            .map(protectedResourcePo -> {
                ProtectedResource resource = new ProtectedResource();
                BeanUtils.copyProperties(protectedResourcePo, resource);
                return resource;
            }).collect(Collectors.toList());
    }

    @Override
    public List<ProtectedResource> getResourceListBySubTypeList(List<String> subTypeList) {
        if (VerifyUtil.isEmpty(subTypeList)) {
            return new ArrayList<>();
        }
        LambdaQueryWrapper<ProtectedResourcePo> wrapper = new LambdaQueryWrapper<ProtectedResourcePo>()
            .in(ProtectedResourcePo::getSubType, subTypeList);
        return protectedResourceMapper.selectList(wrapper).stream()
            .map(protectedResourcePo -> {
                ProtectedResource resource = new ProtectedResource();
                BeanUtils.copyProperties(protectedResourcePo, resource);
                return resource;
            }).collect(Collectors.toList());
    }

    @Override
    public List<String> getAllSubTypeList() {
        return protectedResourceMapper.getAllSubTypeList();
    }

    @Override
    @Transactional(rollbackFor = Exception.class)
    public void updateCopyUser(UpdateCopyUserObjectReq updateCopyUserObjectReq) {
        UserInnerResponse userInfo = userService.getUserInfoByUserId(updateCopyUserObjectReq.getUserId());
        if (!userService.isDPAdminAndCustomizedUser(updateCopyUserObjectReq.getUserId())) {
            log.error("Only the data protection administrator can update the user to which the resource belongs.");
            new LegoCheckedException(CommonErrorCode.USER_NOT_DP_ADMIN);
        }
        // 找到资源对应的各种类型的副本列表
        CopyListParams copyListParams = buildCopyListParams(updateCopyUserObjectReq);
        if (UserTypeEnum.HCS.getValue().equals(userInfo.getUserType())) {
            checkHCSUserQuota(userInfo, copyListParams);
            updateCopyUserForHCS(userInfo, copyListParams);
            return;
        }
        checkUserQuotaExceptHCS(userInfo, copyListParams);
        updateCopyUserExceptHcs(userInfo, copyListParams);
    }

    private CopyListParams buildCopyListParams(UpdateCopyUserObjectReq updateCopyUserObjectReq) {
        CopyListParams copyListParams = new CopyListParams();
        copyListParams.setArchiveCopies(
            copyRestApi.queryCopiesByResourceIdAndGeneratedBy(updateCopyUserObjectReq.getResourceId(),
                Collections.singletonList(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value())));
        copyListParams.setBackupCopies(
            copyRestApi.queryCopiesByResourceIdAndGeneratedBy(updateCopyUserObjectReq.getResourceId(),
                Collections.singletonList(CopyGeneratedByEnum.BY_BACKUP.value())));
        copyListParams.setReplicationCopies(
            copyRestApi.queryCopiesByResourceIdAndGeneratedBy(updateCopyUserObjectReq.getResourceId(),
                Arrays.asList(CopyGeneratedByEnum.BY_REPLICATED.value(),
                    CopyGeneratedByEnum.BY_CASCADED_REPLICATION.value(),
                    CopyGeneratedByEnum.BY_REVERSE_REPLICATION.value())));
        return copyListParams;
    }

    @Override
    public int batchUpdateStatusById(List<String> uuids, int protectStatus) {
        if (VerifyUtil.isEmpty(uuids)) {
            return 0;
        }
        int batchSize = 1000;
        int totalSize = uuids.size();
        int fromIndex = 0;
        while (fromIndex < totalSize) {
            // 获取当前批次的 ID 列表
            int toIndex = Math.min(fromIndex + batchSize, totalSize);
            List<String> subList = uuids.subList(fromIndex, toIndex);

            // 执行批量更新
            LambdaUpdateWrapper<ProtectedResourcePo> wrapper = new LambdaUpdateWrapper<>();
            wrapper.in(ProtectedResourcePo::getUuid, subList);
            wrapper.set(ProtectedResourcePo::getProtectionStatus, protectStatus);
            protectedResourceMapper.update(null, wrapper);
            fromIndex = toIndex;
        }

        return 0;
    }

    private void updateCopyUserForHCS(UserInnerResponse userInfoByUserId, CopyListParams copyListParams) {
        // HCS用户特殊处理
        updateUserQuota(userInfoByUserId.getUserId(), copyListParams.getTotalCopies());
        updateCopyResourceSet(userInfoByUserId.getUserId(), copyListParams.getTotalCopies());
        copyManagerService.updateCopiesUserId(
            copyListParams.getTotalCopies().stream().map(Copy::getUuid).collect(Collectors.toList()),
            userInfoByUserId.getUserId());
    }

    private void checkHCSUserQuota(UserInnerResponse userInfoByUserId, CopyListParams copyListParams) {
        if (!VerifyUtil.isEmpty(copyListParams.getBackupCopies()) && !quotaService.checkIsUserHasEnoughQuota(
            userInfoByUserId.getUserId(), copyListParams.getBackupCopies(), TASK_BACKUP)) {
            log.error("The user type to be modified is {}. The {} quota of HCS is insufficient.",
                UserTypeEnum.HCS.getValue(), TASK_BACKUP);
            throw new LegoCheckedException(CommonErrorCode.USER_QUOTA_NOT_ENOUGH,
                new String[] {userInfoByUserId.getUserType(), TASK_BACKUP});
        }
        if (!VerifyUtil.isEmpty(copyListParams.getReplicationCopies()) && quotaService.checkIsUserHasEnoughQuota(
            userInfoByUserId.getUserId(), copyListParams.getReplicationCopies(), TASK_REPLICATED)) {
            log.error("The user type to be modified is {}. The {} quota of HCS is insufficient.",
                UserTypeEnum.HCS.getValue(), TASK_REPLICATED);
            throw new LegoCheckedException(CommonErrorCode.USER_QUOTA_NOT_ENOUGH,
                new String[] {userInfoByUserId.getUserType(), TASK_REPLICATED});
        }
        if (!VerifyUtil.isEmpty(copyListParams.getArchiveCopies()) && quotaService.checkIsUserHasEnoughQuota(
            userInfoByUserId.getUserId(), copyListParams.getArchiveCopies(), TASK_ARCHIVE)) {
            log.error("The user type to be modified is {}. The {} quota of HCS is insufficient.",
                UserTypeEnum.HCS.getValue(), TASK_ARCHIVE);
            throw new LegoCheckedException(CommonErrorCode.USER_QUOTA_NOT_ENOUGH,
                new String[] {userInfoByUserId.getUserType(), TASK_ARCHIVE});
        }
    }

    private void updateCopyUserExceptHcs(UserInnerResponse userInfoByUserId, CopyListParams copyListParams) {
        updateUserQuota(userInfoByUserId.getUserId(), copyListParams.getTotalCopies());
        updateCopyResourceSet(userInfoByUserId.getUserId(), copyListParams.getTotalCopies());
        copyManagerService.updateCopiesUserId(
            copyListParams.getTotalCopies().stream().map(Copy::getUuid).collect(Collectors.toList()),
            userInfoByUserId.getUserId());
    }

    private void checkUserQuotaExceptHCS(UserInnerResponse userInfoByUserId, CopyListParams copyListParams) {
        if (!VerifyUtil.isEmpty(copyListParams.getArchiveCopies()) && !quotaService.checkIsUserHasEnoughQuota(
            userInfoByUserId.getUserId(), copyListParams.getArchiveCopies(), TASK_ARCHIVE)) {
            log.error("The user type to be modified is {}. The {} quota is insufficient.",
                userInfoByUserId.getUserType(), TASK_ARCHIVE);
            throw new LegoCheckedException(CommonErrorCode.USER_QUOTA_NOT_ENOUGH,
                new String[] {userInfoByUserId.getUserType(), TASK_ARCHIVE});
        }
        if (!VerifyUtil.isEmpty(copyListParams.getBackAndReplicationCopies())
            && !quotaService.checkIsUserHasEnoughQuota(userInfoByUserId.getUserId(),
            copyListParams.getBackAndReplicationCopies(), TASK_BACKUP)) {
            log.error("The user type to be modified is {}. The {} quota is insufficient.",
                userInfoByUserId.getUserType(), TASK_BACKUP);
            throw new LegoCheckedException(CommonErrorCode.USER_QUOTA_NOT_ENOUGH,
                new String[] {userInfoByUserId.getUserType(), TASK_BACKUP});
        }
    }

    private void updateUserQuota(String userId, List<Copy> copyList) {
        Map<String, List<Copy>> copyListGroupedByGeneratedBy = copyList.stream()
            .filter(copy -> copy.getUserId() != null)
            .collect(Collectors.groupingBy(Copy::getGeneratedBy));
        copyListGroupedByGeneratedBy.forEach((generatedBy, copyGeneratedByList) -> {
            Map<String, List<Copy>> copyListGroupedByUserId = copyGeneratedByList.stream()
                .filter(copy -> copy.getUserId() != null)
                .collect(Collectors.groupingBy(Copy::getUserId));
            copyListGroupedByUserId.forEach((user, copies) -> {
                String totalSize = String.valueOf(copies.stream().mapToLong(copy -> {
                    JSONObject properties = JSONObject.fromObject(copy.getProperties());
                    String quotaStr = properties.getString(SIZE, DATA_SIZE_DEFAULT_VALUE);
                    return Long.parseLong(quotaStr);
                }).sum());
                if (!userService.isSysAdmin(user)) {
                    quotaService.updateUserUsedQuota(user, null, generatedBy, REDUCE, totalSize);
                }
            });
            String totalSize = String.valueOf(copyGeneratedByList.stream().mapToLong(copy -> {
                JSONObject properties = JSONObject.fromObject(copy.getProperties());
                String quotaStr = properties.getString(SIZE, DATA_SIZE_DEFAULT_VALUE);
                return Long.parseLong(quotaStr);
            }).sum());
            quotaService.updateUserUsedQuota(userId, null, generatedBy, INCREASE, totalSize);
        });
    }

    private void updateCopyResourceSet(String userId, List<Copy> copyList) {
        Map<String, List<String>> userIdToUuidMap = copyList.stream()
            .filter(copy -> copy.getUserId() != null)
            .collect(Collectors.groupingBy(Copy::getUserId, Collectors.mapping(Copy::getUuid, Collectors.toList())));
        String currentUserDomainId = domainService.getDomainInfoByUserId(userId).getUuid();
        // 若原先副本属于系统管理员则不需要删除其资源集关系
        userIdToUuidMap.forEach((useId, copyIdList) -> {
            if (!userService.isSysAdmin(useId)) {
                // 资源集删除
                List<ResourceSetRoleResponse> resourceSetRoleResponses
                    = resourceSetService.queryResourceSetRoleRelationListByUserIdWithoutCheck(useId);
                resourceSetResourceService.deleteResourceSetWithResource(copyIdList, resourceSetRoleResponses.stream()
                    .map(ResourceSetRoleResponse::getResourceSetId)
                    .collect(Collectors.toList()));
                // 域删除
                domainResourceSetService.batchDeleteDomainResourceObjectRelation(
                    domainService.getDomainInfoByUserId(useId).getUuid(), copyIdList, StringUtils.EMPTY);
            }
        });
        // 添加到新用户的默认资源集中
        ResourceSetBo defaultResourceSetByDomainId = resourceSetService.getDefaultResourceSetByDomainId(
            currentUserDomainId);
        List<ResourceSetResourceEntity> resourceSetResourceEntityList = copyList.stream()
            .map(
                copy -> ResourceSetRelationHelper.buildResourceSetResourceEntity(defaultResourceSetByDomainId.getUuid(),
                    copy.getUuid(), ResourceSetTypeEnum.COPY.getType(), ResourceSetTypeEnum.COPY.getType(),
                    Boolean.TRUE))
            .collect(Collectors.toList());
        resourceSetResourceService.batchCreateResourceSetRelation(resourceSetResourceEntityList);
        // 添加域关系
        List<DomainResourceObjectEntity> domainResourceObjectEntities = copyList.stream()
            .map(copy -> buildDomainResourceObjectEntity(currentUserDomainId, copy.getUuid(), ResourceSetTypeEnum.COPY,
                ResourceSetTypeEnum.COPY.getType()))
            .collect(Collectors.toList());
        domainResourceSetService.batchAddRelations(domainResourceObjectEntities);
    }

    private DomainResourceObjectEntity buildDomainResourceObjectEntity(String domainId, String resourceObjectId,
        ResourceSetTypeEnum type, String scopeModule) {
        DomainResourceObjectEntity domainResourceObjectEntity = new DomainResourceObjectEntity();
        domainResourceObjectEntity.setDomainId(domainId);
        domainResourceObjectEntity.setResourceObjectId(resourceObjectId);
        domainResourceObjectEntity.setType(type);
        domainResourceObjectEntity.setScopeModule(scopeModule);
        return domainResourceObjectEntity;
    }
}
