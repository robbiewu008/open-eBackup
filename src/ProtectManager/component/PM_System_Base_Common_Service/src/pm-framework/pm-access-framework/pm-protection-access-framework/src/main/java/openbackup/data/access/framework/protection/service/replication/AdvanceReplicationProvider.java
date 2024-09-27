/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.protection.service.replication;

import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.EXTERNAL_SYSTEM_ID;
import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.REPLICATION_TARGET_MODE;
import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.STORAGE_ID;
import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.STORAGE_INFO;
import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.STORAGE_TYPE;
import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.USERNAME;
import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.USER_INFO;

import com.huawei.oceanprotect.base.cluster.sdk.entity.TargetCluster;
import com.huawei.oceanprotect.base.cluster.sdk.enums.StorageUnitTypeEnum;
import com.huawei.oceanprotect.base.cluster.sdk.service.ArrayTargetClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import com.huawei.oceanprotect.base.cluster.sdk.service.TargetClusterService;
import com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants;
import com.huawei.oceanprotect.sla.common.constants.SlaConstants;
import com.huawei.oceanprotect.sla.domain.factory.SlaDomainFactory;
import com.huawei.oceanprotect.sla.domain.model.entity.SlaEntity;
import com.huawei.oceanprotect.sla.infrastructure.repository.SlaRepositoryImpl;
import com.huawei.oceanprotect.sla.sdk.api.ReplicationSlaUserService;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.ReplicationSlaUser;
import com.huawei.oceanprotect.sla.sdk.dto.RetentionDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.enums.BackupStorageTypeEnum;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.enums.ReplicationMode;
import com.huawei.oceanprotect.sla.sdk.enums.RetentionType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.osac.service.StorageEntityRestApiService;
import com.huawei.oceanprotect.system.sdk.enums.SwitchNameEnum;
import com.huawei.oceanprotect.system.sdk.enums.SwitchStatusEnum;
import com.huawei.oceanprotect.system.sdk.service.SystemSwitchInternalService;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.config.achive.DmeResponse;
import openbackup.data.access.client.sdk.api.framework.dmc.DmcCopyService;
import openbackup.data.access.client.sdk.api.framework.dmc.model.CopyDetail;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.DmeReplicationRestApi;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.model.DmeReplicateRequest;
import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.common.constants.CopyConstants;
import openbackup.data.access.framework.core.common.util.CopyInfoBuilder;
import openbackup.data.access.framework.protection.common.constants.ReplicationJobLabelConstant;
import openbackup.data.access.framework.protection.common.constants.ReplicationPolicyKeyConstant;
import openbackup.data.access.framework.protection.dto.CopyReplicationMetadata;
import openbackup.data.access.framework.protection.dto.DmeRemoteDevicesRequestDto;
import openbackup.data.access.framework.protection.listener.v1.replication.ReplicateContext;
import openbackup.data.access.framework.protection.service.job.JobLogRecorder;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.StorageRepositoryCreateService;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyReplicationImport;
import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.replication.IReplicateContext;
import openbackup.data.protection.access.provider.sdk.replication.ReplicationProvider;
import openbackup.system.base.bean.DeviceNetworkInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.DateFormatConstant;
import openbackup.system.base.common.constants.ProtocolPortConstant;
import openbackup.system.base.common.enums.TimeUnitEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.asserts.PowerAssert;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.PageQueryRestApi;
import openbackup.system.base.sdk.accesspoint.model.DmeLocalDevice;
import openbackup.system.base.sdk.accesspoint.model.DmeRemoteDevice;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.job.util.JobUpdateUtil;
import openbackup.system.base.sdk.protection.QosCommonRestApi;
import openbackup.system.base.sdk.protection.emuns.SlaPolicyTypeEnum;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.QosBo;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.BackupUnitVo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.DeployTypeService;

import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.joda.time.DateTime;
import org.joda.time.format.DateTimeFormat;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.context.annotation.Primary;
import org.springframework.stereotype.Component;
import org.springframework.util.Assert;

import java.net.URI;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

/**
 * Abstract DME Replication Provider
 *
 * @author l00272247
 * @since 2020-11-19
 */
@Component
@Slf4j
@Primary
public class AdvanceReplicationProvider implements ReplicationProvider {
    private static final int DEFAULT_STORAGE_PORT = 8088;

    private static final String START_REPLICATE_TIME = "start_replicate_time";

    private static final String TEMP_USER_INFO = "temp_";

    private static final String REPLICATION_TARGET_TYPE = "replication_target_type";

    private static final String ALL_COPY = "1";

    private static final int BASIC_DISK_PORT = -1;

    private static final int DEFAULT_PORT = 8088;

    Set<String> dirResources = new HashSet<>(
        Arrays.asList(ResourceSubTypeEnum.SQL_SERVER.getType(), ResourceSubTypeEnum.OPENGAUSS.getType(),
            ResourceSubTypeEnum.DAMENG.getType(), ResourceSubTypeEnum.GAUSSDBT.getType(),
            ResourceSubTypeEnum.REDIS.getType(), ResourceSubTypeEnum.CLICK_HOUSE.getType(),
            ResourceSubTypeEnum.GAUSSDB_DWS.getType()));

    @Autowired
    private DmcCopyService dmcCopyService;

    @Autowired
    private DmeReplicationRestApi dmeReplicationRestApi;

    @Autowired
    private ClusterInternalApi clusterInternalApi;

    @Autowired
    private QosCommonRestApi qosCommonRestApi;

    @Autowired
    private JobCenterRestApi jobCenterRestApi;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private SlaQueryService slaQueryService;

    @Autowired
    private TaskRepositoryManager taskRepositoryManager;

    @Autowired
    private SystemSwitchInternalService systemSwitchInternalService;

    @Autowired
    private SlaRepositoryImpl slaRepository;

    @Autowired
    private ClusterNativeApi clusterNativeApi;

    @Autowired
    private BackupStorageApi backupStorageApi;

    @Autowired
    private ClusterQueryService clusterQueryService;

    @Autowired
    private AuthNativeApi authNativeApi;

    @Autowired
    private StorageUnitService storageUnitService;

    @Autowired
    private ArrayTargetClusterService arrayTargetClusterService;

    @Autowired
    @Qualifier("defaultClusterServiceImpl")
    private ClusterService clusterService;

    @Autowired
    private TargetClusterService targetClusterService;

    @Autowired
    private StorageRepositoryCreateService storageRepositoryCreateService;

    @Autowired
    private ReplicationSlaUserService replicationSlaUserService;

    @Autowired
    private JobLogRecorder jobLogRecorder;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private StorageEntityRestApiService storageEntityRestApiService;

    /**
     * replicate backup object
     *
     * @param ctx context
     */
    @ExterAttack
    @Override
    public void replicate(IReplicateContext ctx) {
        if (!(ctx instanceof ReplicateContext)) {
            return;
        }
        ReplicateContext context = (ReplicateContext) ctx;
        DmeReplicateRequest request = new DmeReplicateRequest();
        request.setLocalEsn(clusterNativeApi.getCurrentEsn());
        ResourceEntity resource = context.getResourceEntity();
        try {
            String resourceId = ResourceSubTypeEnum.IMPORT_COPY.getType().equals(resource.getSubType())
                ? resource.getName()
                : resource.getUuid();
            request.setResourceId(resourceId);
            PolicyBo policy = context.getPolicy();
            setReplicateTime(request, policy);
            request.setApplicationType(resource.getSubType());
            BackupObject backupObject = context.getBackupObject();
            request.setJobRequestId(backupObject.getRequestId());
            request.setTaskId(backupObject.getTaskId());
            checkReplicationSlaUser(policy, backupObject.getTaskId());
            // 初始化本地设备
            initLocalDevice(context, request, resourceId);
            initRemoteDevice(context, request, resourceId);
            initQos(context, request);
            // 初始化metadata
            initMetadata(context, request);
            String jobId = context.getContext().get("job_id");
            updateJob(jobId);
            request.setEnableCompress(
                policy.getBooleanFormExtParameters(ReplicationPolicyKeyConstant.LINK_COMPRESSION).orElse(false));
            request.setEnableDeduplication(
                policy.getBooleanFormExtParameters(ReplicationPolicyKeyConstant.LINK_DEDUPLICATION).orElse(false));
            // 设置复制链路加密开关
            request.setEnableEncryption(getSwitchByName(SwitchNameEnum.REPLICATION_LINK_ENCRYPTION));
            request.setSameChainCopies(context.getSameChainCopies());
            initCopyFormat(request, context);

            executeReplica(request, context);
            log.info("AdvanceReplication job send to dme success, job_id:{}, start time: {}", jobId,
                request.getStartReplicateTime());
            UpdateJobRequest deliverReq = JobUpdateUtil.getDeliverReq();
            // 在extendStr中加入复制目标位置,hcs跨云复制跳过
            updateReplicationDestination(context, deliverReq);
            jobCenterRestApi.updateJob(jobId, deliverReq);
        } finally {
            // 清理密码
            if (request.getLocalDevices() != null) {
                request.getLocalDevices().forEach(localDevice -> StringUtil.clean(localDevice.getPassword()));
            }
            if (request.getRemoteDevice() != null) {
                request.getRemoteDevice().forEach(dmeRemoteDevice -> {
                    StringUtil.clean(dmeRemoteDevice.getPasswordPm());
                    StringUtil.clean(dmeRemoteDevice.getTokenPM());
                });
            }
        }
    }

    private void updateReplicationDestination(ReplicateContext context, UpdateJobRequest deliverReq) {
        PolicyBo policy = context.getPolicy();
        JsonNode extParameters = policy.getExtParameters();
        int replicationMode = policy.getIntegerFormExtParameters(REPLICATION_TARGET_MODE,
                ReplicationMode.EXTRA.getValue());
        if (ReplicationMode.HCS.getValue() == replicationMode) {
            return;
        }
        if (ReplicationMode.INTRA.getValue() == replicationMode) {
            initReplicationDestinationWithIntra(extParameters, deliverReq);
        } else {
            initReplicationDestinationWithExtra(context, deliverReq);
        }
    }

    private void initReplicationDestinationWithIntra(JsonNode extParameters, UpdateJobRequest deliverReq) {
        // 如果是开了并行存储开关的备份存储单元组，位置信息显示备份存储单元组的名称，其他场景不返回
        if (isStorageUnitGroup(extParameters)) {
            String storageId = extParameters.get(STORAGE_INFO).get(STORAGE_ID).textValue();
            NasDistributionStorageDetail storageUnitGroup = backupStorageApi.getDetail(storageId);
            if (storageUnitGroup.isHasEnableParallelStorage()) {
                JSONObject repDestination = new JSONObject();
                repDestination.put("rep_destination", storageUnitGroup.getName());
                deliverReq.setExtendStr(repDestination.toString());
            }
        }
    }

    private void initReplicationDestinationWithExtra(ReplicateContext context, UpdateJobRequest deliverReq) {
        String replicationDestination = getReplicationDestinationByContext(context);
        JSONObject repDestination = new JSONObject();
        repDestination.put("rep_destination", replicationDestination);
        deliverReq.setExtendStr(repDestination.toString());
    }

    private boolean isStorageUnitGroup(JsonNode extParameters) {
        return extParameters.has(STORAGE_INFO) && extParameters.get(STORAGE_INFO).has(STORAGE_ID)
                && BackupConstant.BACKUP_EXT_PARAM_STORAGE_UNIT_GROUP_VALUE
                .equals(extParameters.get(STORAGE_INFO).get(STORAGE_TYPE).textValue());
    }

    private String getReplicationDestinationByContext(ReplicateContext context) {
        PolicyBo policy = context.getPolicy();
        JsonNode extParameters = policy.getExtParameters();
        int clusterId = extParameters.get(EXTERNAL_SYSTEM_ID).asInt();
        String jobId = context.getContext().get("job_id");
        log.info("targetCluster id: {}, jobId: {}", clusterId, jobId);
        if (!extParameters.has(STORAGE_INFO)) {
            return Strings.EMPTY;
        }
        String storageName = StringUtils.EMPTY;
        if (isStorageUnitGroup(extParameters)) {
            TargetCluster targetCluster = clusterQueryService.getTargetClusterById(clusterId);
            String storageId = extParameters.get(STORAGE_INFO).get(STORAGE_ID).textValue();
            NasDistributionStorageDetail storageUnitGroup = arrayTargetClusterService
                    .getTargetDistribution(targetCluster, storageId).orElse(null);
            if (!VerifyUtil.isEmpty(storageUnitGroup)) {
                storageName = storageUnitGroup.getName();
            }
            log.info("storage unit group name: {}, jobId: {}", storageName, jobId);
        } else {
            String storageUnitId = extParameters.get(STORAGE_INFO).get(STORAGE_ID).textValue();
            Map<String, String> queryParams = Collections.singletonMap("id", storageUnitId);
            List<StorageUnitVo> storageUnitVo = getStorageUnitVos(context.getTargetCluster(), queryParams);
            if (!VerifyUtil.isEmpty(storageUnitVo)) {
                storageName = storageUnitVo.get(0).getName();
            }
            log.info("storage unit name: {}, jobId: {}", storageName, jobId);
        }
        return storageName;
    }

    private void executeReplica(DmeReplicateRequest request, ReplicateContext context) {
        DmeResponse<String> result;
        PolicyBo policy = context.getPolicy();
        String localStorageType = getStringFromJsonNode(policy.getExtParameters(), "local_storage_type");
        if (StringUtils.equals(StorageUnitTypeEnum.BASIC_DISK.getType(), localStorageType)) {
            String unitId = context.getContext().get("unit");
            Optional<StorageUnitVo> storageUnit = storageUnitService.getStorageUnitById(unitId);
            if (!storageUnit.isPresent()) {
                throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Local storage not exist.");
            }
            URI dmeServiceUri = storageEntityRestApiService.getDmeServiceUri(storageUnit.get().getDeviceId(),
                ProtocolPortConstant.REPLICATION_PORT);
            result = dmeReplicationRestApi.replicate(dmeServiceUri, request);
        } else {
            result = dmeReplicationRestApi.replicate(request);
        }
        if (result.getExceptionIfError().isPresent()) {
            LegoCheckedException exception = result.getExceptionIfError().get();
            log.info("Failed to execute replication task, error code is {}, job id: {}", exception.getErrorCode(),
                request.getTaskId());
            if (exception.getErrorCode() == CommonErrorCode.BACKUP_CLUSTER_NOT_ALL_ONLINE) {
                throw new LegoCheckedException(CommonErrorCode.BACKUP_CLUSTER_NOT_ALL_ONLINE,
                    new String[] {String.valueOf(2)}, "The number of online storage units is less than 2.");
            }
            throw exception;
        }
    }

    private void initCopyFormat(DmeReplicateRequest request, ReplicateContext context) {
        String lastCopyFormat = context.getContext().get("copy_type");
        log.info("get last copy format: {}", lastCopyFormat);
        request.setCopyFormat(
            lastCopyFormat != null ? Integer.parseInt(lastCopyFormat) : CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
    }

    private void initLocalDevice(ReplicateContext context, DmeReplicateRequest request, String resourceId) {
        BackupObject backupObject = context.getBackupObject();
        List<DmeLocalDevice> dmeLocalDevices = buildLocalDevices(backupObject, resourceId, context);
        PolicyBo policy = context.getPolicy();
        dmeLocalDevices.forEach(dmeLocalDevice -> dmeLocalDevice
                .setLocalStorageType(getStringFromJsonNode(policy.getExtParameters(), "local_storage_type")));
        request.setLocalDevices(dmeLocalDevices);
        String copyId = context.getContext().get("copy_id");
        if (StringUtils.isNotBlank(copyId)) {
            log.info("Reverse replicate copy: {}", copyId);
            // 有副本id为反向复制，填充存储库
            request.getLocalDevices().addAll(getCopyDevices(copyId));
            request.setCopyId(copyId);
        }
    }

    private void setReplicateTime(DmeReplicateRequest request, PolicyBo policy) {
        JsonNode targetType = policy.getExtParameters().get(REPLICATION_TARGET_TYPE);
        if (targetType != null && ALL_COPY.equals(targetType.asText())) {
            log.info("Resource {} is all copy,will set replicateTime.", request.getTaskId());
            JsonNode replicateTime = policy.getExtParameters().get(START_REPLICATE_TIME);
            if (replicateTime == null || StringUtils.isBlank(replicateTime.asText()) || "null".equals(
                replicateTime.asText())) {
                request.setStartReplicateTime(
                    TimeUnit.MILLISECONDS.toSeconds(policy.getSchedule().getStartTime().getTime()));
            } else {
                DateTime dateTime = DateTime.parse(replicateTime.asText(),
                    DateTimeFormat.forPattern(DateFormatConstant.DATE_TIME));
                request.setStartReplicateTime(TimeUnit.MILLISECONDS.toSeconds(dateTime.getMillis()));
            }
        }
    }

    private List<DmeLocalDevice> getCopyDevices(String copyId) {
        Map<String, Object> condition = new HashMap<>();
        condition.put("uuid", copyId);
        BasePage<Copy> basePage = copyRestApi.queryCopies(0, 1, condition);
        String storageId = basePage.getItems().get(0).getStorageId();
        if (VerifyUtil.isEmpty(storageId)) {
            return new ArrayList<>();
        }
        return taskRepositoryManager.buildTargetRepositories(storageId, SlaPolicyTypeEnum.BACKUP.getName())
            .stream()
            .map(this::toDmeLocalDevice)
            .collect(Collectors.toList());
    }

    private boolean getSwitchByName(SwitchNameEnum switchNameEnum) {
        return SwitchStatusEnum.ON.equals(systemSwitchInternalService.queryByName(switchNameEnum).getStatus());
    }

    private void initMetadata(ReplicateContext context, DmeReplicateRequest request) {
        PolicyBo policy = context.getPolicy();
        String srcUserId = context.getResourceEntity().getUserId();
        CopyReplicationMetadata metadata = new CopyReplicationMetadata();
        RMap<String, String> rMap = redissonClient.getMap(context.getRequestId(), StringCodec.INSTANCE);
        // 修改原复制策略保留时间
        updateReplicationRetentionAfterUpgrade(rMap);
        JSONObject resource = JSONObject.fromObject(rMap.get("resource"));
        resource.put("user_id", srcUserId);
        metadata.setResource(resource);
        metadata.setSla(JSONObject.fromObject(rMap.get("sla")));
        metadata.setJobType(rMap.get("job_type"));
        JsonNode extParameters = policy.getExtParameters();
        // 放入对端username
        String dpUserName = null;
        if (extParameters != null && extParameters.has(USER_INFO) && extParameters.get(USER_INFO).has(USERNAME)) {
            dpUserName = extParameters.get(USER_INFO).get(USERNAME).textValue();
        }
        TargetClusterVo cluster = context.getTargetCluster();
        String username = VerifyUtil.isEmpty(dpUserName) ? cluster.getUsername() : dpUserName;
        metadata.setUsername(username);
        metadata.setReplicationPolicy(policy);
        request.setMetadata(JSONObject.fromObject(metadata).toString());
    }

    private void updateReplicationRetentionAfterUpgrade(RMap<String, String> rmap) {
        String slaStr = rmap.get("sla");
        if (StringUtils.isBlank(slaStr)) {
            log.debug("reverse replication has no sla");
            return;
        }
        JSONObject slaObject = JSONObject.fromObject(slaStr);
        SlaDto sla = slaObject.toBean(SlaDto.class);
        if (sla.getApplication() == null || !dirResources.contains(sla.getApplication().getType())) {
            return;
        }
        List<PolicyDto> backupPolicy = sla.getPolicyList()
            .stream()
            .filter(e -> PolicyType.BACKUP.equals(e.getType()))
            .collect(Collectors.toList());
        List<PolicyDto> replicationPolicyList = sla.getPolicyList()
            .stream()
            .filter(e -> PolicyType.REPLICATION.equals(e.getType()))
            .collect(Collectors.toList());
        PolicyDto replicationPolicy = replicationPolicyList.get(0);
        JsonNode extParameters = replicationPolicy.getExtParameters();
        if (extParameters.has(ExtParamsConstants.HAS_UPGRADE) && extParameters.get(ExtParamsConstants.HAS_UPGRADE)
            .asBoolean()) {
            return;
        }
        RetentionDto maxRetention = getMaxRetention(backupPolicy);
        log.info("get backup max retention type: {}, retention duration:{}", maxRetention.getRetentionType(),
            maxRetention.getRetentionDuration());
        sla.getPolicyList()
            .stream()
            .filter(e -> PolicyType.REPLICATION.equals(e.getType()))
            .forEach(e -> e.setRetention(maxRetention));
        UpdateSlaCommand slaBase = new UpdateSlaCommand();
        slaBase.setType(sla.getType());
        slaBase.setName(sla.getName());
        slaBase.setUserId(sla.getUserId());
        slaBase.setApplication(sla.getApplication().getType());
        slaBase.setPolicyList(sla.getPolicyList());
        slaBase.setUuid(sla.getUuid());
        SlaEntity modifiedSla = SlaDomainFactory.createSla(slaBase);
        modifiedSla.setUuid(sla.getUuid());
        slaRepository.update(modifiedSla);
        rmap.put("sla", JsonUtil.json(sla));
    }

    private RetentionDto getMaxRetention(List<PolicyDto> backupPolicy) {
        Optional<PolicyDto> permanentBackupPolicy = backupPolicy.stream()
            .filter(e -> !RetentionType.TEMPORARY.equals(e.getRetention().getRetentionType()))
            .findFirst();
        RetentionDto retention;
        if (permanentBackupPolicy.isPresent()) {
            retention = permanentBackupPolicy.get().getRetention();
        } else {
            // 找出保留时间最长的备份策略
            long currentTimeMillis = System.currentTimeMillis();
            PolicyDto maxRetentionPolicy = backupPolicy.stream().max((o1, o2) -> {
                Date date1 = CopyInfoBuilder.computeExpirationTime(currentTimeMillis,
                    TimeUnitEnum.getByUnit(o1.getRetention().getDurationUnit()),
                    o1.getRetention().getRetentionDuration());
                Date date2 = CopyInfoBuilder.computeExpirationTime(currentTimeMillis,
                    TimeUnitEnum.getByUnit(o2.getRetention().getDurationUnit()),
                    o2.getRetention().getRetentionDuration());
                return date1.compareTo(date2);
            }).orElseThrow(() -> new LegoCheckedException("backup policy not found"));
            retention = maxRetentionPolicy.getRetention();
        }
        return retention;
    }

    private void initRemoteDevice(ReplicateContext context, DmeReplicateRequest request, String resourceId) {
        int replicationMode = context.getPolicy()
            .getIntegerFormExtParameters(REPLICATION_TARGET_MODE, ReplicationMode.EXTRA.getValue());
        log.info("replication mode:{}", replicationMode);
        TargetClusterVo cluster = context.getTargetCluster();
        PolicyBo policy = context.getPolicy();
        List<DmeRemoteDevice> dmeRemoteDeviceList;
        if (SlaConstants.EXTRA_REPLICATION_MODE.contains(replicationMode)) { // 跨域复制
            dmeRemoteDeviceList = fillDmeRemoteDeviceWhenExtra(cluster, policy);
        } else {
            request.setIntra(true);
            JsonNode storageInfo = policy.getExtParameters().get(ExtParamsConstants.STORAGE_INFO);
            dmeRemoteDeviceList = generateIntraDeviceList(storageInfo, resourceId);
        }
        String remoteStorageType = getStringFromJsonNode(policy.getExtParameters(), "remote_storage_type");
        dmeRemoteDeviceList.forEach(device -> device.setRemoteStorageType(remoteStorageType));
        request.setRemoteDevice(dmeRemoteDeviceList);
    }

    private List<DmeRemoteDevice> fillDmeRemoteDeviceWhenExtra(TargetClusterVo cluster, PolicyBo policy) {
        if (deployTypeService.isE1000()) {
            return fillDmeRemoteDeviceWhenExtraForD8(cluster, policy);
        }
        PowerAssert.notEmpty(cluster.getClusterDetailInfo().getAllMemberClustersDetail(),
            () -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "No remote device found"));
        List<DmeRemoteDevice> dmeRemoteDevices = obtainAllMemberRemoteDevice(cluster, policy);
        log.info("get remote member detail success ,member count: {}", dmeRemoteDevices.size());
        return dmeRemoteDevices;
    }

    private List<DmeRemoteDevice> fillDmeRemoteDeviceWhenExtraForD8(TargetClusterVo cluster, PolicyBo policy) {
        JsonNode storageInfo = policy.getExtParameters().get(ExtParamsConstants.STORAGE_INFO);
        String storageType = storageInfo.get(ExtParamsConstants.STORAGE_TYPE).asText();
        String storageId = storageInfo.get(ExtParamsConstants.STORAGE_ID).asText();
        if (BackupStorageTypeEnum.BACKUP_STORAGE_UNIT_GROUP.getValue().equals(storageType)) {
            return getDmeRemoteDevicesByStorageGroup(cluster, storageId);
        } else {
            return getDmeRemoteDevicesByStorageUnit(cluster, storageId);
        }
    }

    private List<DmeRemoteDevice> getDmeRemoteDevicesByStorageUnit(TargetClusterVo cluster, String storageId) {
        Map<String, String> queryParams = new HashMap<>();
        queryParams.put("id", storageId);
        StorageUnitVo storageUnitVo = getStorageUnitVos(cluster, queryParams).get(0);
        DmeRemoteDevice device = new DmeRemoteDevice();
        String deviceEsn = storageUnitVo.getDeviceId();
        List<ClusterDetailInfo> allMemberClustersDetail = cluster.getClusterDetailInfo().getAllMemberClustersDetail();
        ClusterDetailInfo storage = allMemberClustersDetail.stream()
                .filter(clusterDetailInfo -> StringUtils.equals(deviceEsn,
                        clusterDetailInfo.getStorageSystem().getStorageEsn()))
                .findFirst()
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Storage not exist."));
        device.setEsn(deviceEsn);
        device.setBackupSoftwareEsn(cluster.getEsn());
        device.setPort(DEFAULT_STORAGE_PORT);
        device.setPortPm(cluster.getPort());
        device.setDeployType(storage.getSourceClusters().getDeployType());
        device.setNetworkInfo(JSONObject
                .fromObject(cluster.getClusterDetailInfo().getStorageSystem().getDeviceNetworkInfo()).toString());
        device.setMgrIpList(cluster.getMgrIpList());
        device.setUserNamePm(cluster.getUsername());
        device.setPasswordPm(cluster.getPassword());
        device.setTokenPM(StringUtils.EMPTY);
        device.setPoolId(storageUnitVo.getPoolId());
        return Collections.singletonList(device);
    }

    private List<DmeRemoteDevice> getDmeRemoteDevicesByStorageGroup(TargetClusterVo cluster, String storageId) {
        NasDistributionStorageDetail detail = backupStorageApi.getDetail(storageId);
        return detail.getUnitList().stream().map(backupUnitVo -> {
            DmeRemoteDevice device = new DmeRemoteDevice();
            String deviceEsn = backupUnitVo.getDeviceId();
            device.setEsn(deviceEsn);
            device.setPort(DEFAULT_STORAGE_PORT);
            device.setPortPm(backupUnitVo.getBackupClusterVo().getPmPort());
            device.setDeployType(backupUnitVo.getBackupClusterVo().getDeployType());
            device.setMgrIpList(Arrays.asList(backupUnitVo.getBackupClusterVo().getClusterIp().split(",")));
            device.setNetworkInfo((JSONObject
                    .fromObject(cluster.getClusterDetailInfo().getStorageSystem().getDeviceNetworkInfo()).toString()));
            device.setUserNamePm("sysadmin");
            String token = authNativeApi.generateClusterAdminToken(-1);
            device.setTokenPM(token);
            device.setPoolId(backupUnitVo.getPoolId());
            return device;
        }).collect(Collectors.toList());
    }

    private List<DmeRemoteDevice> generateIntraDeviceList(JsonNode extParam, String resourceId) {
        ClusterDetailInfo primaryGroupClustersDetail = clusterQueryService.getPrimaryGroupClustersDetail();
        List<ClusterDetailInfo> allMemberClustersDetail = primaryGroupClustersDetail.getAllMemberClustersDetail();
        Map<String, String> netPlaneMap = allMemberClustersDetail.stream()
            .collect(Collectors.toMap(clusterDetailInfo -> clusterDetailInfo.getStorageSystem().getStorageEsn(),
                clusterDetailInfo -> JSONArray.fromObject(clusterDetailInfo.getTargetClusterVo().getNetplaneInfo())
                    .toString()));
        Map<String, List<String>> mgrIpMap = allMemberClustersDetail.stream()
            .collect(Collectors.toMap(clusterDetailInfo -> clusterDetailInfo.getStorageSystem().getStorageEsn(),
                clusterDetailInfo -> clusterDetailInfo.getTargetClusterVo().getMgrIpList()));
        String token = authNativeApi.generateClusterAdminToken(-1);
        String esn = clusterQueryService.getCurrentClusterEsn();
        String storageType = extParam.get(ExtParamsConstants.STORAGE_TYPE).asText();
        String storageId = extParam.get(ExtParamsConstants.STORAGE_ID).asText();
        DmeRemoteDevicesRequestDto dmeRemoteDevicesRequestDto = DmeRemoteDevicesRequestDto.builder()
                .allMemberClustersDetail(allMemberClustersDetail).storageId(storageId).netPlaneMap(netPlaneMap)
                .mgrIpMap(mgrIpMap).token(token).esn(esn).resourceId(resourceId).build();
        if (BackupStorageTypeEnum.BACKUP_STORAGE_UNIT_GROUP.getValue().equals(storageType)) {
            return getDmeRemoteDevicesByStorage(dmeRemoteDevicesRequestDto);
        } else if (BackupStorageTypeEnum.BACKUP_STORAGE_UNIT.getValue().equals(storageType)) {
            return getDmeRemoteDevicesByStorageUnit(allMemberClustersDetail, storageId, netPlaneMap, mgrIpMap, token);
        } else {
            return getDmeRemoteDevicesWithoutStorage(dmeRemoteDevicesRequestDto);
        }
    }

    private List<DmeRemoteDevice> getDmeRemoteDevicesByStorageUnit(List<ClusterDetailInfo> allMemberClustersDetail,
        String storageId, Map<String, String> netPlaneMap, Map<String, List<String>> mgrIpMap, String token) {
        StorageUnitVo storageUnitVo = storageUnitService.getStorageUnitById(storageId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Storage unit not exist"));
        DmeRemoteDevice device = new DmeRemoteDevice();
        String deviceEsn = storageUnitVo.getDeviceId();
        ClusterDetailInfo detailInfo = allMemberClustersDetail.stream()
            .filter(clusterDetailInfo -> StringUtils.equals(clusterDetailInfo.getTargetClusterVo().getEsn(), deviceEsn))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "storage unit not found"));
        device.setEsn(deviceEsn);
        device.setPort(DEFAULT_STORAGE_PORT);
        device.setPortPm(detailInfo.getTargetClusterVo().getPort());
        device.setDeployType(detailInfo.getTargetClusterVo().getDeployType());
        device.setNetPlaneInfo(netPlaneMap.get(deviceEsn));
        device.setNetworkInfo(JSONObject.fromObject(detailInfo.getStorageSystem().getDeviceNetworkInfo()).toString());
        device.setMgrIpList(mgrIpMap.get(deviceEsn));
        device.setUserNamePm("sysadmin");
        device.setTokenPM(token);
        device.setPoolId(storageUnitVo.getPoolId());
        return Collections.singletonList(device);
    }

    private List<DmeRemoteDevice> getDmeRemoteDevicesWithoutStorage(DmeRemoteDevicesRequestDto request) {
        return storageUnitService.getAllAvailableStorageUnitForResource(request.getResourceId()).stream()
                .filter(storageUnitVo -> request.getNetPlaneMap().containsKey(storageUnitVo.getDeviceId()))
                .filter(storageUnitVo -> !StringUtils.equals(request.getEsn(), storageUnitVo.getDeviceId()))
                .map(storageUnitVo -> {
                    DmeRemoteDevice device = new DmeRemoteDevice();
                    device.setEsn(storageUnitVo.getDeviceId());
                    device.setPort(DEFAULT_STORAGE_PORT);
                    device.setPortPm(
                            targetClusterService.getTargetClusterByEsn(storageUnitVo.getDeviceId()).getClusterPort());
                    device.setNetPlaneInfo(request.getNetPlaneMap().get(storageUnitVo.getDeviceId()));
                    device.setMgrIpList(request.getMgrIpMap().get(storageUnitVo.getDeviceId()));
                    device.setNetworkInfo(getNetworkInfoFromAllMemberClustersDetail(
                            request.getAllMemberClustersDetail(), storageUnitVo.getDeviceId()));
                    device.setUserNamePm("sysadmin");
                    device.setTokenPM(request.getToken());
                    return device;
                })
                .collect(Collectors.toList());
    }

    private List<DmeRemoteDevice> getDmeRemoteDevicesByStorage(DmeRemoteDevicesRequestDto request) {
        NasDistributionStorageDetail detail = backupStorageApi.getDetail(request.getStorageId());
        String esn = clusterQueryService.getCurrentClusterEsn();
        List<ClusterDetailInfo> allMemberClustersDetail = request.getAllMemberClustersDetail();
        return detail.getUnitList()
            .stream()
            .filter(backupUnitVo -> request.getNetPlaneMap().containsKey(backupUnitVo.getDeviceId()))
            .filter(backupUnitVo -> !StringUtils.equals(esn, backupUnitVo.getDeviceId()))
            .filter(backupUnitVo -> getAvailableStorageUnit(request.getResourceId()).contains(backupUnitVo.getUnitId()))
            .map(backupUnitVo -> {
                DmeRemoteDevice device = new DmeRemoteDevice();
                String deviceEsn = backupUnitVo.getDeviceId();
                device.setEsn(deviceEsn);
                device.setPort(DEFAULT_STORAGE_PORT);
                device.setPortPm(backupUnitVo.getBackupClusterVo().getPmPort());
                device.setDeployType(backupUnitVo.getBackupClusterVo().getDeployType());
                device.setNetPlaneInfo(request.getNetPlaneMap().get(deviceEsn));
                device.setMgrIpList(request.getMgrIpMap().get(deviceEsn));
                device.setNetworkInfo(getNetworkInfoFromAllMemberClustersDetail(allMemberClustersDetail, deviceEsn));
                device.setUserNamePm("sysadmin");
                device.setTokenPM(request.getToken());
                device.setPoolId(backupUnitVo.getPoolId());
                return device;
            })
            .collect(Collectors.toList());
    }

    private List<String> getAvailableStorageUnit(String resourceId) {
        return storageUnitService.getAllAvailableStorageUnitForResource(resourceId)
                .stream().map(StorageUnitVo::getId).collect(Collectors.toList());
    }

    private String getNetworkInfoFromAllMemberClustersDetail(List<ClusterDetailInfo> allMemberClustersDetail,
        String deviceEsn) {
        DeviceNetworkInfo networkInfo = allMemberClustersDetail.stream()
            .map(ClusterDetailInfo::getStorageSystem)
            .filter(storageSystem -> deviceEsn.equals(storageSystem.getStorageEsn()))
            .map(StorageSystemInfo::getDeviceNetworkInfo)
            .findAny()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Unable to find network info"));
        return JSONObject.writeValueAsString(networkInfo);
    }

    private List<DmeLocalDevice> buildLocalDevices(BackupObject backupObject, String resourceId,
            ReplicateContext context) {
        List<DmeLocalDevice> dmeLocalDeviceList = new ArrayList<>();
        DmeLocalDevice localDevice = new DmeLocalDevice();
        // 第一个本地存储信息放在列表中第一个
        if (deployTypeService.isE1000()) {
            buildLocalDevicesForD8(context, dmeLocalDeviceList, localDevice);
        } else {
            dmeLocalDeviceList.add(DmeLocalDevice.build(clusterInternalApi));
        }
        // 查询备份策略里得存储storage_id,目前只支持单个,如果是反向复制则不存在sla
        SlaDto slaDto = slaQueryService.querySlaById(backupObject.getProtectedObject().getSlaId());
        // 反向复制不存在SLA
        if (slaDto == null) {
            log.info("Reverse replication has no sla. Request id: {}", backupObject.getRequestId());
            return dmeLocalDeviceList;
        }
        // 获取资源的全部副本信息
        List<Copy> copies = copyRestApi.queryCopiesByResourceId(resourceId);
        // 获取副本的全部存储id，非空，去重。
        List<String> storageIdList = copies.stream().map(Copy::getStorageId).filter(StringUtils::isNotEmpty).distinct()
                .collect(Collectors.toList());
        // 查询存储库信息
        for (String storageId : storageIdList) {
            try {
                List<StorageRepository> storageRepositories = storageRepositoryCreateService
                        .createRepositoryByStorageUnitGroup(storageId);
                List<DmeLocalDevice> deviceList = storageRepositories.stream().map(this::toDmeLocalDevice)
                        .collect(Collectors.toList());
                dmeLocalDeviceList.addAll(deviceList);
            } catch (LegoCheckedException e) {
                log.warn(String.format(
                        "Unable to build repository by storage unit group id :%s, it may not exist any more.",
                        storageId), ExceptionUtil.getErrorMessage(e));
            }
        }
        return dmeLocalDeviceList;
    }

    private void buildLocalDevicesForD8(ReplicateContext context, List<DmeLocalDevice> dmeLocalDeviceList,
            DmeLocalDevice localDevice) {
        String unit = context.getContext().get("unit");
        fillLocalDevice(dmeLocalDeviceList, localDevice, unit);
    }

    private void fillLocalDevice(List<DmeLocalDevice> dmeLocalDeviceList, DmeLocalDevice localDevice, String unitId) {
        Optional<StorageUnitVo> storageUnit = storageUnitService.getStorageUnitById(unitId);
        if (!storageUnit.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Local storage not exist.");
        }

        ClusterDetailInfo clusterDetail = clusterInternalApi.queryClusterDetails();
        ClusterDetailInfo localCluster = clusterDetail.getAllMemberClustersDetail().stream()
                .filter(clusterDetailInfo -> StringUtils.equals(
                        clusterDetailInfo.getStorageSystem().getStorageEsn(), storageUnit.get().getDeviceId()))
                .findFirst().orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                        "Local storage not exist."));
        localDevice.setPassword(localCluster.getStorageSystem().getPassword());
        localDevice.setUserName(localCluster.getStorageSystem().getUsername());
        int storagePort = localCluster.getStorageSystem().getStoragePort();
        int port = storagePort == BASIC_DISK_PORT ? DEFAULT_PORT : storagePort;
        localDevice.setPort(port);
        localDevice.setEsn(storageUnit.get().getDeviceId());
        localDevice.setMgrIp(clusterDetail.getSourceClusters().getMgrIpList());
        dmeLocalDeviceList.add(localDevice);
    }

    private String getStringFromJsonNode(JsonNode extParameters, String key) {
        Assert.notNull(extParameters, "Ext param can not be null!");
        JsonNode localStorageTypeJson = extParameters.get(key);
        return VerifyUtil.isEmpty(localStorageTypeJson) ? StringUtils.EMPTY : localStorageTypeJson.asText();
    }

    private DmeLocalDevice toDmeLocalDevice(StorageRepository storageRepository) {
        DmeLocalDevice dmeLocalDevice = new DmeLocalDevice();
        dmeLocalDevice.setMgrIp(Arrays.asList(storageRepository.getEndpoint().getIp().split(",")));
        dmeLocalDevice.setUserName(storageRepository.getExtendAuth().getAuthKey());
        dmeLocalDevice.setPassword(storageRepository.getExtendAuth().getAuthPwd());
        dmeLocalDevice.setPort(storageRepository.getEndpoint().getPort());
        dmeLocalDevice.setEsn(storageRepository.getId());
        return dmeLocalDevice;
    }

    private void updateJob(String jobId) {
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        updateJobRequest.setStatus(JobStatusEnum.RUNNING);
        jobCenterRestApi.updateJob(jobId, updateJobRequest, false);
    }

    private void initQos(ReplicateContext context, DmeReplicateRequest request) {
        JsonNode qosNode = context.getPolicy().getExtParameters().findPath("qos_id");
        if (qosNode.isMissingNode()) {
            return;
        }
        String qosId = qosNode.asText();
        if (VerifyUtil.isEmpty(qosId) || "null".equals(qosId)) {
            return;
        }
        // 设置Qos限速
        QosBo qos = qosCommonRestApi.queryQos(qosId);
        request.setQos(qos.getSpeedLimit());
    }

    private DmeRemoteDevice obtainRemoteDevice(ClusterDetailInfo clusterDetailInfo, String groupId,
        StorageUnitVo storageUnitVo) {
        TargetClusterVo cluster = clusterDetailInfo.getTargetClusterVo();
        DmeRemoteDevice remoteDevice = obtainDeviceWithCluster(cluster, groupId, storageUnitVo.getPoolId());
        remoteDevice.setNetworkInfo(
            JSONObject.fromObject(clusterDetailInfo.getStorageSystem().getDeviceNetworkInfo()).toString());
        return remoteDevice;
    }


    private DmeRemoteDevice obtainDeviceWithCluster(TargetClusterVo cluster, String storageUnitGroupIdForDws,
        String poolId) {
        DmeRemoteDevice remoteDevice = new DmeRemoteDevice();
        remoteDevice.setPort(DEFAULT_STORAGE_PORT);
        remoteDevice.setPortPm(cluster.getPort());
        remoteDevice.setEsn(cluster.getEsn());
        remoteDevice.setUserNamePm(cluster.getUsername());
        remoteDevice.setPasswordPm(cluster.getPassword());
        remoteDevice.setMgrIpList(cluster.getMgrIpList());
        remoteDevice.setNetPlaneInfo(JSONArray.fromObject(cluster.getNetplaneInfo()).toString());
        remoteDevice.setStorageId(storageUnitGroupIdForDws);
        remoteDevice.setDeployType(cluster.getDeployType());
        remoteDevice.setPoolId(poolId);
        return remoteDevice;
    }

    private String getStorageUnitGroupIdForDws(PolicyBo policy) {
        Object externalStorageIdObject = JSONObject.fromObject(policy.getExtParameters())
                .get(CopyPropertiesKeyConstant.KEY_REPLICATE_EXTERNAL_REPOSITORY_ID);
        String storageId = "";
        if (externalStorageIdObject instanceof String) {
            storageId = (String) externalStorageIdObject;
        }
        return storageId;
    }

    private String getPoolId(TargetClusterVo cluster, JsonNode extParameters) {
        String esn = cluster.getEsn();
        Map<String, String> queryParams = new HashMap<>();
        if (!extParameters.has(ExtParamsConstants.STORAGE_INFO)) { // 取对端esn的默认存储单元
            queryParams.put("deviceId", esn);
        } else { // 取storage_info里面的存储单元id去查poolId
            JsonNode storageInfo = extParameters.get(STORAGE_INFO);
            queryParams.put("id", storageInfo.get(STORAGE_ID).textValue());
        }
        List<StorageUnitVo> remoteStorageUnits = getStorageUnitVos(cluster, queryParams);
        return remoteStorageUnits.get(0).getPoolId();
    }

    private List<StorageUnitVo> getStorageUnitVos(TargetClusterVo cluster, Map<String, String> queryParams) {
        List<StorageUnitVo> remoteStorageUnits =
            getRemoteStorageUnit(queryParams, Integer.parseInt(cluster.getClusterId())).getRecords();
        if (VerifyUtil.isEmpty(remoteStorageUnits)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "The storage unit belongs to the target cluster not exist.");
        }
        return remoteStorageUnits;
    }

    private PageListResponse<StorageUnitVo> getRemoteStorageUnit(Map<String, String> queryParam, Integer clusterId) {
        PageListResponse<StorageUnitVo> response = new PageListResponse<>();
        TargetCluster targetCluster = clusterQueryService.getTargetClusterById(clusterId);
        try {
            response = arrayTargetClusterService.getStorageUnitInfo(targetCluster, queryParam, 0, 1);
            return response;
        } catch (LegoUncheckedException e) {
            log.error("get all dp users failed.", ExceptionUtil.getErrorMessage(e));
        }
        return response;
    }

    private void checkReplicationSlaUser(PolicyBo policy, String jobId) {
        JsonNode extParameters = policy.getExtParameters();
        JsonNode clusterId = extParameters.get(EXTERNAL_SYSTEM_ID);
        if (!extParameters.has(USER_INFO)) { // 1.5升级场景不校验
            return;
        }
        if (VerifyUtil.isEmpty(clusterId)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "ClusterId is null.");
        }
        QueryWrapper<ReplicationSlaUser> wrapper = new QueryWrapper<>();
        wrapper.eq("policy_id", policy.getUuid());
        ReplicationSlaUser slaUser = replicationSlaUserService.queryReplicationSlaUser(wrapper)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.USER_OR_PASSWORD_IS_INVALID,
                "User name or password error."));
        try {
            if (!clusterService.verifyUsernameAndPassword(Integer.valueOf(clusterId.asText()),
                    slaUser.getUsername(), slaUser.getPassword())) {
                // 记录恢复任务错误信息
                jobLogRecorder.recordJobStepWithError(jobId, ReplicationJobLabelConstant.CHECK_REPLICATION_USER_FAILED,
                        CommonErrorCode.USER_OR_PASSWORD_IS_INVALID, null);
                throw new LegoCheckedException(CommonErrorCode.USER_OR_PASSWORD_IS_INVALID,
                        "User name or password error.");
            }
        } finally {
            StringUtil.clean(slaUser.getPassword());
            if (slaUser.getUuid().startsWith(TEMP_USER_INFO)) {
                log.warn("Manual replication user info will be delete now. Job id: {}", jobId);
                replicationSlaUserService.deleteReplicationSlaUserById(slaUser.getUuid());
            }
        }
    }

    private List<DmeRemoteDevice> obtainAllMemberRemoteDevice(TargetClusterVo cluster, PolicyBo policy) {
        // 从端是多集群时，要把选中的从端的单元（组）中的包含的节点信息发给从端，同时指定存储池ID
        List<DmeRemoteDevice> dmeRemoteDeviceList = new ArrayList<>();
        List<ClusterDetailInfo> allMemberClustersDetail = cluster.getClusterDetailInfo().getAllMemberClustersDetail();
        List<StorageUnitVo> unitList = getAllRemoteUnitByPolicy(cluster, policy);
        PowerAssert.notNull(unitList, () -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
            "Unable find any storage unit on remote device"));
        Set<String> remoteDeviceInPolicy =
            unitList.stream().map(StorageUnitVo::getDeviceId).collect(Collectors.toSet());
        String storageUnitGroupIdForDws = getStorageUnitGroupIdForDws(policy);
        allMemberClustersDetail.forEach(detailInfo -> {
            String esn = detailInfo.getTargetClusterVo().getEsn();
            if (!remoteDeviceInPolicy.contains(esn)) {
                return;
            }
            StorageUnitVo unitVo = unitList.stream()
                .filter(storageUnitVo -> esn.equals(storageUnitVo.getDeviceId()))
                .findAny()
                .orElse(new StorageUnitVo());
            detailInfo.getTargetClusterVo().setPassword(cluster.getPassword());
            detailInfo.getTargetClusterVo().setUsername(cluster.getUsername());
            dmeRemoteDeviceList.add(obtainRemoteDevice(detailInfo, storageUnitGroupIdForDws, unitVo));
        });
        return dmeRemoteDeviceList;
    }

    private List<StorageUnitVo> getAllRemoteUnitByPolicy(TargetClusterVo cluster, PolicyBo policy) {
        if (!policy.getExtParameters().has(STORAGE_INFO)
            || !policy.getExtParameters().get(STORAGE_INFO).has(STORAGE_ID)) {
            log.info("Remote unit or group not specified. All units available.");
            return getRemoteStorageUnit(Collections.emptyMap(),
                Integer.parseInt(cluster.getClusterId())).getRecords();
        }
        JsonNode storageInfo = policy.getExtParameters().get(STORAGE_INFO);
        String storageId = storageInfo.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_ID_KEY).textValue();
        if (BackupConstant.BACKUP_EXT_PARAM_STORAGE_UNIT_GROUP_VALUE
            .equals(storageInfo.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY).textValue())) {
            return arrayTargetClusterService
                .getTargetDistribution(targetClusterService.getTargetClusterByEsn(cluster.getEsn()), storageId)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "Replication backup storage unit group not exists."))
                .getUnitList()
                .stream()
                .map(BackupUnitVo::toStorageUnitVo)
                .collect(Collectors.toList());
        } else {
            Map<String, String> queryParams = new HashMap<>();
            queryParams.put("id", storageId);
            return getStorageUnitVos(cluster, queryParams);
        }
    }

    /**
     * build copy info
     *
     * @param copy copy
     * @param importParam importParam
     */
    @Override
    public void buildCopyProperties(CopyInfoBo copy, CopyReplicationImport importParam) {
        log.info("The target receives metadata information, importParam.metadata:{}", importParam.getMetadata());
        JSONObject properties = JSONObject.fromObject(importParam.getProperties());
        String backupId = properties.getString("backup_id");
        copy.setUuid(backupId);
        JSONObject copyProperties = buildCopyProperties(copy, backupId);
        copyProperties.put(CopyConstants.BACKUP_ID, backupId);
        copyProperties.put(CopyPropertiesKeyConstant.KEY_REPLICATE_COUNT,
            properties.getInt(CopyPropertiesKeyConstant.KEY_REPLICATE_COUNT, 1));
        copy.setProperties(copyProperties.toString());
        int features = CopyFeatureEnum.setAndGetFeatures(
            Arrays.asList(CopyFeatureEnum.RESTORE, CopyFeatureEnum.INSTANT_RESTORE, CopyFeatureEnum.MOUNT));
        copy.setFeatures(features);
        copy.setDeletable(true);
        int backupType = properties.getInt("backup_type");
        copy.setBackupType(backupType);
        int sourceCopyType = properties.getInt("source_copy_type", backupType);
        copy.setSourceCopyType(sourceCopyType);
    }

    /**
     * build copy properties
     *
     * @param copy copy
     * @param backupId backup id
     * @return copy properties
     */
    protected JSONObject buildCopyProperties(CopyInfoBo copy, String backupId) {
        return new JSONObject();
    }

    /**
     * check copy whether exist
     *
     * @param chainId chainId
     * @param timestamp timestamp
     * @return 待入库副本是否已存在
     */
    @Override
    public boolean checkCopyWhetherExist(String chainId, long timestamp) {
        long count = PageQueryRestApi.get(copyRestApi::queryCopies)
            .count(new JSONObject().set("chain_id", chainId).set("timestamp", timestamp + "000000"));
        if (count > 0) {
            log.info("copy(chain-id:{}, timestamp:{}) was already imported.", chainId, timestamp);
            return true;
        }
        return false;
    }

    /**
     * get dmc copy info
     *
     * @param backupId backup id
     * @param subType sub type
     * @return copy detail
     */
    protected CopyDetail getDmcCopyInfo(String backupId, String subType) {
        DmeResponse<CopyDetail> copyResponse = dmcCopyService.queryCopyById(backupId, subType);
        return copyResponse.getCheckedData();
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.IMPORT_COPY.getType().equals(object);
    }
}
