package openbackup.access.framework.resource.service.impl;

import openbackup.access.framework.resource.dto.DeliverTaskReq;
import openbackup.access.framework.resource.service.AgentBusinessService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.DeliverTaskStatusDto;
import openbackup.data.access.client.sdk.api.framework.dee.DeeLiveMountRestApi;
import openbackup.data.access.client.sdk.api.framework.dee.model.OcLiveMountTaskReq;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * AgentBusinessService实现类
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-27
 */
@Service
@Slf4j
public class AgentBusinessServiceImpl implements AgentBusinessService {
    private static final String EXTEND_INFO = "extendInfo";

    private static final String SCRIPT = "script";

    private static final String TYPE = "type";

    // 内置agent的key
    private static final String INTERNAL_AGENT_KEY = "scenario";

    // 内置agent的value
    private static final String INTERNAL_AGENT_VALUE = "1";

    private static final int SIZE = 100;

    private final ResourceService resourceService;

    private final JobService jobService;

    private final CopyRestApi copyRestApi;

    private final AgentUnifiedService agentUnifiedService;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private DeeLiveMountRestApi deeLiveMountRestApi;

    public AgentBusinessServiceImpl(ResourceService resourceService, JobService jobService, CopyRestApi copyRestApi,
        AgentUnifiedService agentUnifiedService) {
        this.resourceService = resourceService;
        this.jobService = jobService;
        this.copyRestApi = copyRestApi;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public void deliverTaskStatus(DeliverTaskReq deliverTaskReq) {
        JobBo jobBo = jobService.queryJob(deliverTaskReq.getTaskId());
        String script = "";
        if (deployTypeService.isCyberEngine()) {
            // 安全一体机适配共享路径恢复：反馈任务结果或任务尝试间未上报时调用dee接口实现deeLiveMountRestApi
            log.info("OceanCyber liveMount task[{}] manual report {} status.",
                deliverTaskReq.getTaskId(), deliverTaskReq.getStatus());
            OcLiveMountTaskReq liveMountTaskReq = new OcLiveMountTaskReq();
            liveMountTaskReq.setTaskId(deliverTaskReq.getTaskId());
            liveMountTaskReq.setRequestId(deliverTaskReq.getTaskId());
            liveMountTaskReq.setStatus(deliverTaskReq.getStatus());
            deeLiveMountRestApi.updateLiveMountTask(liveMountTaskReq);
        } else {
            script = getScriptFromJob(jobBo);
            for (DeliverTaskReq.AgentDto agent : deliverTaskReq.getAgents()) {
                ProtectedEnvironment environment = resourceService.queryHostByEndpoint(agent.getEndpoint());
                DeliverTaskStatusDto deliverTaskStatusDto = new DeliverTaskStatusDto();
                deliverTaskStatusDto.setStatus(deliverTaskReq.getStatus());
                deliverTaskStatusDto.setTaskId(deliverTaskReq.getTaskId());
                deliverTaskStatusDto.setScript(script);
                agentUnifiedService.deliverTaskStatus(jobBo.getSourceSubType(), deliverTaskStatusDto,
                    environment.getEndpoint(), environment.getPort());
            }
        }
    }

    /**
     * 查询内置agent
     *
     * @return internal agent list
     */
    @Override
    public List<Endpoint> queryInternalAgents() {
        Map<String, Object> filter = new HashMap<>();
        filter.put(TYPE, ResourceTypeEnum.HOST.getType());
        filter.put(INTERNAL_AGENT_KEY, INTERNAL_AGENT_VALUE);
        PageListResponse<ProtectedResource> response = sessionService.call(() -> resourceService.query(0, SIZE, filter),
            Constants.Builtin.ROLE_SYS_ADMIN);
        return response.getRecords()
            .stream()
            .map(this::findAgent)
            .filter(Optional::isPresent)
            .map(Optional::get)
            .collect(Collectors.toList());
    }

    @Override
    public List<ProtectedEnvironment> queryInternalAgentEnv() {
        Map<String, Object> filter = new HashMap<>();
        filter.put(TYPE, ResourceTypeEnum.HOST.getType());
        filter.put(INTERNAL_AGENT_KEY, INTERNAL_AGENT_VALUE);
        ResourceQueryParams params = new ResourceQueryParams();
        params.setShouldDecrypt(true);
        params.setPage(0);
        params.setSize(SIZE);
        params.setConditions(filter);
        params.setShouldIgnoreOwner(true);
        PageListResponse<ProtectedResource> response = resourceService.query(params);
        return response.getRecords()
                .stream()
                .filter(resource -> resource instanceof ProtectedEnvironment)
                .map(resource -> (ProtectedEnvironment) resource)
                .collect(Collectors.toList());
    }

    private Optional<Endpoint> findAgent(ProtectedResource protectedResource) {
        if (protectedResource instanceof ProtectedEnvironment) {
            ProtectedEnvironment env = (ProtectedEnvironment) protectedResource;
            return Optional.of(new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()));
        }
        return Optional.empty();
    }

    private String getScriptFromJob(JobBo jobBo) {
        String script = null;
        String sourceId = jobBo.getSourceId();
        ProtectedResource protectedResource = resourceService.getBasicResourceById(sourceId).orElse(null);
        if (protectedResource != null) {
            script = protectedResource.getExtendInfoByKey(SCRIPT);
        }
        if (VerifyUtil.isEmpty(script)) {
            log.warn("Can not get script from resource. resource id: {}", sourceId);
            Copy copy = copyRestApi.queryCopyByID(jobBo.getCopyId());
            script = getScriptFromCopy(copy);
        }
        if (VerifyUtil.isEmpty(script)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Script can not be found.");
        }
        return script;
    }

    private String getScriptFromCopy(Copy copy) {
        JsonNode jsonNode = JsonUtil.read(copy.getResourceProperties(), JsonNode.class);
        JsonNode path = jsonNode.path(EXTEND_INFO).path(SCRIPT);
        return path.toString();
    }
}
