package openbackup.access.framework.resource.service;

import openbackup.access.framework.resource.ResourceCertConstant;
import openbackup.access.framework.resource.util.ResourceCertAlarmUtil;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.CyberEngineResourceService;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentDeleteProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.model.AgentTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.model.ResourceExtendInfoKeyConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.errors.ResourceLockErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.pack.lock.Lock;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.query.SessionService;

import com.huawei.oceanprotect.system.base.label.dao.LabelResourceServiceDao;
import com.huawei.oceanprotect.system.base.schedule.common.enums.ScheduleType;
import com.huawei.oceanprotect.system.base.schedule.service.SchedulerService;
import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.sdk.schedule.model.Schedule;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;
import com.huawei.oceanprotect.system.sdk.service.SystemConfigService;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.quartz.SchedulerException;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.nio.charset.Charset;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

/**
 * 受保护环境服务实现类
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-15
 */
@Slf4j
@Service
public class ProtectedEnvironmentServiceImpl implements ProtectedEnvironmentService {
    private static final String PLUGIN_TYPE = "PLUGIN";
    private static final String RESOURCE_LOCK_KEY = "/resource/lock/";
    private static final List<String> AGENT_UPGRADE_STATUS =
            Collections.unmodifiableList(
                    Arrays.asList(
                            LinkStatusEnum.AGENT_STATUS_QUEUED.getStatus().toString(),
                            LinkStatusEnum.AGENT_STATUS_UPDATING.getStatus().toString()));

    @Autowired
    private ProviderManager manager;

    @Autowired
    private SchedulerService scheduleService;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private LockService lockService;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private CommonAlarmService commonAlarmService;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private CyberEngineResourceService cyberEngineResourceService;

    @Autowired
    private ProtectedResourceRepository protectedResourceRepository;

    @Autowired
    private SystemConfigService systemConfigService;

    @Autowired
    private LabelResourceServiceDao labelResourceServiceDao;

    /**
     * 扫描受保护环境
     *
     * @param environment 受保护环境
     * @return environment uuid
     */
    @Override
    public String register(ProtectedEnvironment environment) {
        try {
            registerCheck(environment);
            // 暂时使用IP地址+环境的子类型构造uuid，后续要由受保护环境的uuid需要由插件提供。
            if (VerifyUtil.isEmpty(environment.getUuid())) {
                String envIdentity = environment.getEndpoint() + environment.getSubType();
                environment.setUuid(UUID.nameUUIDFromBytes(envIdentity.getBytes(Charset.defaultCharset())).toString());
            }
            String uuid = saveEnvironment(environment);
            submitScanJob(environment, true);
            log.info("Start to delete use_old! environment :{}", environment.getUuid());
            protectedResourceRepository.deleteProtectResourceExtendInfoByResourceId(environment.getUuid(),
                Constants.USE_OLD_PRIVATE);
            return uuid;
        } finally {
            log.info("clean environment password.");
            StringUtil.clean(environment.getPassword());
        }
    }

    @Override
    public ActionResult[] checkProtectedEnvironment(ProtectedEnvironment protectedEnvironment) {
        return resourceService.check(protectedEnvironment, true);
    }

    @Override
    public PageListResponse<ProtectedResource> browse(BrowseEnvironmentResourceConditions environmentConditions) {
        ProtectedEnvironment environment = this.getEnvironmentById(environmentConditions.getEnvId());
        EnvironmentProvider provider = manager.findProvider(EnvironmentProvider.class,
            StringUtils.isNotBlank(environmentConditions.getResourceSubType())
                ? environmentConditions.getResourceSubType()
                : environment.getSubType());

        return provider.browse(environment, environmentConditions);
    }

    private void tryLockAndRun(Runnable runnable, String envId, long time, boolean isStrict) {
        Lock lock = lockService.createSQLDistributeLock(RESOURCE_LOCK_KEY + envId);
        boolean isDone = lock.tryLockAndRun(time, TimeUnit.SECONDS, runnable);
        if (isStrict && !isDone) {
            throw new LegoCheckedException(deployTypeService.isCyberEngine()
                ? ResourceLockErrorCode.OCEAN_CYBER_RESOURCE_ALREADY_LOCKED
                : ResourceLockErrorCode.RESOURCE_ALREADY_LOCKED, new String[] {envId},
                "resource is locked. id: " + envId);
        } else {
            log.debug("try lock and run: {}, strict: {}, envId: {}", isDone, isStrict, envId);
        }
        if (!isDone) {
            log.warn("can not acquire lock and run, envId: {}", envId);
        }
    }

    @Override
    public ProtectedEnvironment getEnvironmentById(String envId) {
        log.info("getEnvironmentById={}", envId);
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(envId);
        if (resOptional.isPresent() && resOptional.get() instanceof ProtectedEnvironment) {
            return (ProtectedEnvironment) resOptional.get();
        }
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected environment is not exists!");
    }

    @Override
    public Optional<ProtectedEnvironment> getBasicEnvironmentById(String envId) {
        Optional<ProtectedResource> resOptional = resourceService.getBasicResourceById(envId);
        return resOptional.filter(resource -> resource instanceof ProtectedEnvironment)
                .map(resource -> (ProtectedEnvironment) resource);
    }

    @Override
    @Transactional(rollbackFor = Exception.class)
    public void deleteEnvironmentById(String envId) {
        Optional<ProtectedResource> resOptional = resourceService.getBasicResourceById(envId);
        ProtectedEnvironment environment;
        if (resOptional.isPresent() && resOptional.get() instanceof ProtectedEnvironment) {
            environment = (ProtectedEnvironment) resOptional.get();
        } else {
            return;
        }

        if (AGENT_UPGRADE_STATUS.contains(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(environment))) {
            log.info("Upgrading agent can not be deleted.");
            return;
        }
        Boolean isCheckSuccess = Optional.ofNullable(
                providerManager.findProvider(ProtectedEnvironmentDeleteProvider.class, environment.getSubType(), null))
                .map(provider -> provider.check(environment)).orElse(true);
        if (!isCheckSuccess) {
            return;
        }
        tryLockAndRun(() -> deleteEnvironment(environment), envId, 1, true);
    }

    private void deleteEnvironment(ProtectedEnvironment environment) {
        EnvironmentProvider provider = manager.findProvider(EnvironmentProvider.class, environment.getSubType());
        provider.remove(environment);
        if (deployTypeService.isCyberEngine()) {
            cyberEngineResourceService.deleteEnvironment(environment.getUuid());
        } else {
            resourceService.delete(new String[] {environment.getUuid()});
        }
        afterDelete(environment);
        try {
            scheduleService.removeJob(environment.getUuid());
            // 删除环境对应标签
            labelResourceServiceDao.deleteByResourceObjectIdsAndLabelIds(
                Collections.singletonList(environment.getUuid()),
                StringUtils.EMPTY);
        } catch (SchedulerException e) {
            log.warn("Remove job failed!", e);
        }
    }

    private void afterDelete(ProtectedEnvironment environment) {
        // 资源删除后清理cert告警
        log.info("after delete clear alarm. resource id: {}", environment.getUuid());
        commonAlarmService.clearAlarm(
            ResourceCertAlarmUtil.genResourceCertExpiredAlarm(ResourceCertConstant.CRL_EXPIRED_ID,
                environment.getName(), environment.getSubType(), environment.getUuid()));
        commonAlarmService.clearAlarm(
            ResourceCertAlarmUtil.genResourceCertExpiredAlarm(ResourceCertConstant.CERT_EXPIRED_ID,
                environment.getName(), environment.getSubType(), environment.getUuid()));
        // 插件后置处理
        Optional.ofNullable(
                providerManager.findProvider(ProtectedEnvironmentDeleteProvider.class, environment.getSubType(), null))
                .ifPresent(provider -> provider.afterDelete(environment));
    }

    @Override
    public void refreshEnvironment(String envId) {
    }

    @Override
    @Transactional(rollbackFor = Exception.class)
    public void updateEnvironment(ProtectedEnvironment environment) {
        ProtectedEnvironment oldEnv = this.getEnvironmentById(environment.getUuid());
        String name = oldEnv.getName();
        // 更新前后名字不一样，需要检查名称是否重复
        if (environment.getName() != null && !Objects.equals(environment.getName(), name)) {
            checkName(environment);
        }
        ProtectedEnvironment environmentData = new ProtectedEnvironment();
        BeanUtils.copyProperties(environment, environmentData);
        Authentication authentication = environment.getAuth();
        // 初始化未加密的鉴权信息
        environmentData.setAuth(
                resourceService.mergeAuthentication(new Authentication[] {authentication, oldEnv.getAuth()}));
        EnvironmentProvider provider = manager.findProvider(EnvironmentProvider.class, environment.getSubType(), null);
        if (provider != null) {
            BeanTools.hold(() -> provider.register(environmentData), environmentData,
                    new String[] {"uuid", "type", "subType"});
        }
        resourceService.update(new ProtectedResource[] {environmentData});

        afterUpdate(environment, oldEnv);
    }

    private void afterUpdate(ProtectedEnvironment environment, ProtectedEnvironment oldEnv) {
        // 更新名称或者更新auth里面的extendInfo，都去清除告警
        if (!VerifyUtil.isEmpty(environment.getName()) || (!VerifyUtil.isEmpty(environment.getAuth())
            && !VerifyUtil.isEmpty(environment.getAuth().getExtendInfo()))) {
            log.info("after update clear alarm. resource id: {}", oldEnv.getUuid());
            commonAlarmService.clearAlarm(
                ResourceCertAlarmUtil.genResourceCertExpiredAlarm(ResourceCertConstant.CRL_EXPIRED_ID, oldEnv.getName(),
                    oldEnv.getSubType(), environment.getUuid()));
            commonAlarmService.clearAlarm(
                ResourceCertAlarmUtil.genResourceCertExpiredAlarm(ResourceCertConstant.CERT_EXPIRED_ID,
                    oldEnv.getName(), oldEnv.getSubType(), environment.getUuid()));
        }
    }

    @Override
    public boolean hasSameEnvironmentInDb(ProtectedEnvironment environment) {
        Optional<ProtectedResource> resource = resourceService.getResourceById(environment.getUuid());

        // 如果数据库没有相同uuid的环境，直接返回
        if (!resource.isPresent()) {
            log.info("Has no old environment {}.", environment.getUuid());
            return false;
        }
        if (resource.get() instanceof ProtectedEnvironment) {
            ProtectedEnvironment oldEnvironment = (ProtectedEnvironment) resource.get();
            ProtectedEnvironment updatedEnvironment = buildUpdatedEnvironment(environment, oldEnvironment);
            log.info(
                    "updated environment {}, ip={}, port={}",
                    updatedEnvironment.getUuid(),
                    updatedEnvironment.getEndpoint(),
                    updatedEnvironment.getPort());
            this.updateEnvironment(updatedEnvironment);
            log.info("Start to delete manualUninstallation! environment :{}", environment.getUuid());
            protectedResourceRepository.deleteProtectResourceExtendInfoByResourceId(environment.getUuid(),
                Constants.MANUAL_UNINSTALLATION);
            // 升级失败场景，不清理use_old_private
            String updateVersion = oldEnvironment.getExtendInfoByKey("agentUpgradeableVersion");
            if (StringUtils.isEmpty(updateVersion) || StringUtils.equals(updateVersion,
                updatedEnvironment.getVersion())) {
                log.info("Start to delete use_old! environment :{}", environment.getUuid());
                protectedResourceRepository.deleteProtectResourceExtendInfoByResourceId(environment.getUuid(),
                    Constants.USE_OLD_PRIVATE);
            }
        }
        submitScanJob(environment, true, true);
        return true;
    }

    @Override
    public void updateInternalAgentAfterSystemDataRecovery(ProtectedEnvironment newEnv) {
        Map<String, String> newEnvExtendInfo = newEnv.getExtendInfo();
        String newEnvScenario = newEnvExtendInfo.get(ResourceExtendInfoKeyConstants.EXT_INFO_SCENARIO);
        if (!StringUtils.equals(newEnvScenario, AgentTypeEnum.INTERNAL_AGENT.getValue())) {
            return;
        }

        Optional<ProtectedResource> oldAgent = resourceService.getBasicResourceById(newEnv.getUuid());
        if (oldAgent.isPresent()) {
            return;
        }

        oldAgent = queryNeedUpdateAgent(newEnv);
        oldAgent.ifPresent(oldAgentResource -> updateOldInternalAgentInfo(newEnv, oldAgentResource));
    }

    private void updateOldInternalAgentInfo(ProtectedEnvironment newEnv, ProtectedResource oldResource) {
        log.info("System recovery scene, old internal agent uuid: {},  endpoint:{}, "
            + "new internal agent uuid: {}, endpoint: {}", oldResource.getUuid(), oldResource.getEndpoint(),
            newEnv.getUuid(), newEnv.getEndpoint());
        newEnv.setRootUuid(oldResource.getRootUuid());
        newEnv.setUuid(oldResource.getUuid());
        // version 同步
        newEnv.getExtendInfo().put(ResourceExtendInfoKeyConstants.RECOVER_REGISTER_VERSION,
            systemConfigService.getConfigValue(ResourceExtendInfoKeyConstants.SYSTEM_RECOVER_VERSION));
    }

    private Optional<ProtectedResource> queryNeedUpdateAgent(ProtectedEnvironment newEnv) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("type", newEnv.getType());
        conditions.put("subType", newEnv.getSubType());
        conditions.put(ResourceExtendInfoKeyConstants.EXT_INFO_SCENARIO, AgentTypeEnum.INTERNAL_AGENT.getValue());
        List<ProtectedResource> internalAgentList = resourceService.query(false, 0, 100, conditions).getRecords();
        Optional<ProtectedResource> agentSameIp = internalAgentList.stream()
            .filter(agent -> agent.getEndpoint().equals(newEnv.getEndpoint()))
            .findAny();
        if (agentSameIp.isPresent()) {
            return isVersionSmallerThanSystem(agentSameIp.get()) ? agentSameIp : Optional.empty();
        }
        return internalAgentList.stream().filter(this::isVersionSmallerThanSystem).findAny();
    }

    private boolean isVersionSmallerThanSystem(ProtectedResource resource) {
        return queryAgentRecoverVersion(resource) < querySystemRecoverVersion();
    }

    private int querySystemRecoverVersion() {
        String systemRecoverVersion = systemConfigService.getConfigValue(
            ResourceExtendInfoKeyConstants.SYSTEM_RECOVER_VERSION);
        return StringUtils.isEmpty(systemRecoverVersion) ? 0 : Integer.parseInt(systemRecoverVersion);
    }

    private int queryAgentRecoverVersion(ProtectedResource resource) {
        String registerVersion = resource.getExtendInfoByKey(ResourceExtendInfoKeyConstants.RECOVER_REGISTER_VERSION);
        return StringUtils.isEmpty(registerVersion) ? 0 : Integer.parseInt(registerVersion);
    }

    @Override
    public void checkHasSameEndpointEnvironment(ProtectedEnvironment environment) {
        Map<String, Object> conditions = getQueryConditions(environment);
        List<ProtectedResource> resources = resourceService.query(false, 0, 1, conditions).getRecords();
        if (resources.isEmpty()) {
            log.info("Has no same endpoint: {} environment: {}", environment.getEndpoint(), environment.getUuid());
            return;
        }

        // 如果endpoint冲突，需要把冲突资源的uuid返回，agent会重新注册
        ProtectedResource oldResource = resources.stream().findFirst().get();
        String uuid = oldResource.getUuid();
        String[] errorParams = new String[] {"uuid", uuid};
        throw new LegoCheckedException(CommonErrorCode.RESOURCE_REPEAT, errorParams,
                "Duplicate environment endpoint exists.");
    }

    private Map<String, Object> getQueryConditions(ProtectedEnvironment environment) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("endpoint", environment.getEndpoint());
        conditions.put("type", environment.getType());
        conditions.put("subType", environment.getSubType());
        return conditions;
    }

    private ProtectedEnvironment buildUpdatedEnvironment(
            ProtectedEnvironment newEnvironment, ProtectedEnvironment oldEnvironment) {
        JSONObject newEnvironmentJson = JSONObject.fromObject(newEnvironment);
        JSONObject oldEnvironmentJson = JSONObject.fromObject(oldEnvironment);

        for (Object key : newEnvironmentJson.keySet()) {
            if (!VerifyUtil.isEmpty(newEnvironmentJson.get(key))) {
                oldEnvironmentJson.set(key, newEnvironmentJson.get(key));
            }
        }
        return JSONObject.toBean(oldEnvironmentJson, ProtectedEnvironment.class);
    }

    private void checkName(ProtectedEnvironment environment) {
        ResourceProvider resourceProvider = providerManager.findProvider(ResourceProvider.class, environment, null);
        if (resourceProvider != null && !resourceProvider.getResourceFeature()
            .isShouldCheckEnvironmentNameDuplicate()) {
            return;
        }
        if (excludeCheckName(environment)) {
            return;
        }
        // 检查相同类型受保护环境的名称是否重复
        Map<String, Object> conditions =
                new JSONObject()
                        .set("name", environment.getName())
                        .set("subType", environment.getSubType())
                        .toMap(Object.class);
        PageListResponse<ProtectedResource> response =
                sessionService.call(() -> resourceService.query(0, 1, conditions), Constants.Builtin.ROLE_SYS_ADMIN);
        long count = response.getRecords().size();
        if (count > 0) {
            throw new LegoCheckedException(CommonErrorCode.DUPLICATE_NAME, "Duplicate environment name exists");
        }
    }

    private boolean excludeCheckName(ProtectedEnvironment environment) {
        return Arrays.asList(ResourceTypeEnum.HOST.getType(), PLUGIN_TYPE).contains(environment.getType());
    }

    private String saveEnvironment(ProtectedEnvironment environment) {
        environment.setRootUuid(Optional.ofNullable(environment.getRootUuid()).orElseGet(environment::getUuid));
        return resourceService.create(new ProtectedResource[] {environment}, true)[0];
    }

    private void registerCheck(ProtectedEnvironment environment) {
        checkName(environment);
        EnvironmentProvider provider = manager.findProvider(EnvironmentProvider.class, environment.getSubType());
        provider.register(environment);
    }

    /**
     * 提交扫描环境定时job
     *
     * @param environment 受保护环境
     * @param isIntervalScan 是否为定时扫描
     */
    private void submitScanJob(ProtectedEnvironment environment, boolean isIntervalScan) {
        Schedule scheduleBean = new Schedule();
        if (isIntervalScan) {
            scheduleBean.setScheduleType(ScheduleType.INTERVAL.getType());
        } else {
            scheduleBean.setScheduleType(ScheduleType.IMMEDIATE.getType());
        }
        if (isIntervalScan) {
            scheduleBean.setInterval(environment.getScanInterval() + "s");
            scheduleBean.setScheduleName(environment.getUuid());
        }
        scheduleBean.setAction(ProtectedEnvironmentListener.SCANNING_ENVIRONMENT_V2);
        scheduleBean.setParams(new JSONObject().set("uuid", environment.getUuid()).toString());
        if (StringUtils.isNotBlank(environment.getStartDate())) {
            // agent(内置 + 外置)注册后延迟5min再去开始执行扫描，避免nginx还未启动导致调接口失败
            scheduleBean.setStartDate(environment.getStartDate());
        }
        try {
            scheduleService.startScheduler(scheduleBean);
        } catch (SchedulerException e) {
            log.error("Submit scan job failed.", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Submit scan job failed.");
        }
    }

    /**
     * 提交扫描环境定时job,可删除原有定时任务
     *
     * @param environment 受保护环境
     * @param isIntervalScan 是否为定时扫描
     * @param shouldDeletePreJob 是否删除原有定时任务
     */
    private void submitScanJob(ProtectedEnvironment environment, boolean isIntervalScan, boolean shouldDeletePreJob) {
        if (shouldDeletePreJob) {
            try {
                scheduleService.removeJob(environment.getUuid());
            } catch (SchedulerException e) {
                log.warn("Remove job failed!host uuid: {}, endPoint: {}", environment.getUuid(),
                    environment.getEndpoint(), ExceptionUtil.getErrorMessage(e));
            }
        }
        submitScanJob(environment, isIntervalScan);
    }
}
