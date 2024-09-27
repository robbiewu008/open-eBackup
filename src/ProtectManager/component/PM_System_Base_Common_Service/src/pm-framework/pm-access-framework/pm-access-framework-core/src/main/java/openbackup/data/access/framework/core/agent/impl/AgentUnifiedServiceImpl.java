package openbackup.data.access.framework.core.agent.impl;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentIqnValidateRequest;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentWwpnInfo;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CleanAgentLogReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CollectAgentLogRsp;
import openbackup.data.access.client.sdk.api.framework.agent.dto.DeliverTaskStatusDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.GetAgentLogCollectStatusRsp;
import openbackup.data.access.client.sdk.api.framework.agent.dto.GetClusterEsnReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Rsp;
import openbackup.data.access.client.sdk.api.framework.agent.dto.PluginsDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ResourceListDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.SupportApplicationDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.SupportPluginDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.UpdateAgentLevelReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.UpdateAgentPluginTypeReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.model.AgentUpdatePluginTypeResult;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.model.AgentCommonParam;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.annotation.Routing;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.model.host.ManagementIp;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.agent.model.AgentUpdateResponse;
import openbackup.system.base.sdk.cert.request.PushUpdateCertToAgentReq;
import openbackup.system.base.service.AvailableAgentManagementDomainService;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.RequestUriUtil;

import feign.FeignException;
import feign.Response;
import feign.codec.Encoder;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.net.Proxy;
import java.net.URI;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * Agent Unified Service impl
 *
 * @author z30009433
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-24
 */
@Slf4j
@Service
public class AgentUnifiedServiceImpl implements AgentUnifiedService, InitializingBean {
    // plugin类型
    private static final String PLUGIN_RESOURCE_TYPE = "Plugin";

    private static final String SUCCESS_ERROR_CODE = "0";

    private boolean isUseProxyForAll;

    private final Encoder encoder;

    @Autowired
    private AvailableAgentManagementDomainService domainService;

    @Autowired
    private DeployTypeService deployTypeService;

    /**
     * Constructor
     *
     * @param encoder 错误嘛
     */
    public AgentUnifiedServiceImpl(Encoder encoder) {
        this.encoder = encoder;
    }

    @Override
    public void afterPropertiesSet() {
        if (deployTypeService.isHyperDetectDeployType() || deployTypeService.isCyberEngine()) {
            log.debug("The deploy type is HyperDetect.No proxy");
            isUseProxyForAll = false;
        } else {
            log.debug("The deploy type is not HyperDetect.");
            isUseProxyForAll = true;
        }
    }

    private AgentUnifiedRestApi getNoRetryAgentUnifiedRestApi(String endPoint) {
        if (isUseProxyForAll) {
            return FeignBuilder.buildNoRetryApi(AgentUnifiedRestApi.class, encoder, true, true,
                getDmeProxy(endPoint));
        }
        return FeignBuilder.buildNoRetryApi(AgentUnifiedRestApi.class, encoder, true, true,
            null);
    }

    private AgentUnifiedRestApi getAgentUnifiedRestApi(String endPoint) {
        if (isUseProxyForAll) {
            return FeignBuilder.buildHttpsTarget(AgentUnifiedRestApi.class, encoder, true, true,
                getDmeProxy(endPoint));
        }
        return FeignBuilder.buildHttpsTarget(AgentUnifiedRestApi.class, encoder, true, true,
            null);
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public PageListResponse<ProtectedResource> getDetailPageList(String appType, String endpoint, Integer port,
        ListResourceV2Req listResourceV2Req, boolean isUseLongTimeApi) {
        return getDetailPageList(new AgentCommonParam(appType, endpoint, port, true), listResourceV2Req,
            isUseLongTimeApi);
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public PageListResponse<ProtectedResource> getDetailPageListNoRetry(String appType, String endpoint, Integer port,
        ListResourceV2Req listResourceV2Req, boolean isUseLongTimeApi) {
        return getDetailPageList(new AgentCommonParam(appType, endpoint, port, false), listResourceV2Req,
            isUseLongTimeApi);
    }

    private PageListResponse<ProtectedResource> getDetailPageList(AgentCommonParam agentCommonParam,
        ListResourceV2Req listResourceV2Req, boolean isUseLongTimeApi) {
        // 1. 获取url
        URI uri = RequestUriUtil.getRequestUri(agentCommonParam.getEndpoint(), agentCommonParam.getPort());
        log.info("Begin to page query resources, appType: {}, apiType: {}, uri: {}.", agentCommonParam.getAppType(),
            isUseLongTimeApi, uri);

        AgentUnifiedRestApi agentApi = (agentCommonParam.isRetry()
            ? getAgentUnifiedRestApi(agentCommonParam.getEndpoint())
            : getNoRetryAgentUnifiedRestApi(agentCommonParam.getEndpoint()));

        // 2. 请求并转换为PageListResponse返回体
        ListResourceV2Rsp listResourceV2Rsp;
        try {
            listResourceV2Rsp = agentApi.listResourceDetailV2(uri, agentCommonParam.getAppType(), listResourceV2Req);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", uri, e);
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
        return convertToPageList(listResourceV2Rsp);
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public AgentDetailDto getDetail(String appType, String endpoint, Integer port, ListResourceReq listResourceReq) {
        return getDetail(new AgentCommonParam(appType, endpoint, port, true), listResourceReq);
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public AgentDetailDto getDetailNoRetry(String appType, String endpoint, Integer port,
        ListResourceReq listResourceReq) {
        return getDetail(new AgentCommonParam(appType, endpoint, port, false), listResourceReq);
    }

    private AgentDetailDto getDetail(AgentCommonParam agentCommonParam, ListResourceReq listResourceReq) {
        URI uri = RequestUriUtil.getRequestUri(agentCommonParam.getEndpoint(), agentCommonParam.getPort());
        try {
            return (agentCommonParam.isRetry() ? getAgentUnifiedRestApi(agentCommonParam.getEndpoint())
                    : getNoRetryAgentUnifiedRestApi(agentCommonParam.getEndpoint())).getDetail(
                uri, agentCommonParam.getAppType(), listResourceReq);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", uri, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        } catch (LegoCheckedException e) {
            ActionResult result = JsonUtil.read(Optional.ofNullable(e.getMessage()).orElse("{}"),
                    ActionResult.class);
            if (VerifyUtil.isEmpty(result.getBodyErr())) {
                throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
            } else {
                throw new LegoCheckedException(Long.parseLong(result.getBodyErr()),
                    Optional.ofNullable(result.getDetailParams())
                        .map(elem -> elem.toArray(new String[0]))
                        .orElse(new String[0]), result.getMessage());
            }
        }
    }

    /**
     * 获取agent的插件信息
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @return 插件转为ProtectedResource后的数据
     */
    @Routing(destinationIp = "#{endpoint}")
    @Override
    public List<ProtectedResource> getPlugins(String endpoint, Integer port) {
        URI uri = RequestUriUtil.getRequestUri(endpoint, port);
        log.debug("Common host environment, get agent plugins, uri: {}", uri);
        PluginsDto plugins;
        try {
            plugins = getAgentUnifiedRestApi(endpoint).getPlugins(uri);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", uri, e);
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
        return convertPluginsToProtectedResources(plugins);
    }

    /**
     * 获取Agent主机信息
     *
     * @param endpoint agent上报环境信息的endpoint
     * @param port agent上报环境信息的port
     * @return agent返回的主机信息
     */
    @Routing(destinationIp = "#{endpoint}")
    @Override
    public HostDto getHost(String endpoint, Integer port) {
        URI uri = RequestUriUtil.getRequestUri(endpoint, port);
        log.debug("Common host environment, get agent host, uri: {}", uri);
        try {
            return getAgentUnifiedRestApi(endpoint).getHost(uri);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", uri, e);
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    /**
     * 获取Agent主机信息
     * 这个接口兼容性最好
     *
     * @param endpoint agent上报环境信息的endpoint
     * @param port agent上报环境信息的port
     * @return agent返回的主机信息
     */
    @Routing(destinationIp = "#{endpoint}")
    @Override
    public HostDto getAgentHost(String endpoint, Integer port) {
        URI uri = RequestUriUtil.getRequestUri(endpoint, port);
        log.debug("Common host environment, get agent host, uri: {}", uri);
        try {
            return getAgentUnifiedRestApi(endpoint).getAgentHost(uri);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", uri, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    @Routing(destinationIp = "#{agent.endpoint}")
    @Override
    public AgentBaseDto checkApplication(ProtectedResource resource, ProtectedEnvironment agent) {
        CheckAppReq checkAppReq = CheckAppReq.buildFrom(resource);
        return check(resource, agent, checkAppReq);
    }

    @Routing(destinationIp = "#{agent.endpoint}")
    @Override
    public AgentBaseDto checkApplicationNoRetry(ProtectedResource resource, ProtectedEnvironment agent) {
        CheckAppReq checkAppReq = CheckAppReq.buildFrom(resource);
        return checkNoRetry(resource, agent, checkAppReq);
    }

    @Routing(destinationIp = "#{agent.endpoint}")
    @Override
    public AgentBaseDto check(ProtectedResource resource, ProtectedEnvironment agent, CheckAppReq checkAppReq) {
        return check(resource, agent, checkAppReq, true);
    }

    @Routing(destinationIp = "#{agent.endpoint}")
    @Override
    public AgentBaseDto check(String subType, ProtectedEnvironment agent, CheckAppReq checkAppReq) {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(subType);
        return check(resource, agent, checkAppReq, true);
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public AgentBaseDto check(String subType, String endpoint, Integer port, CheckAppReq checkAppReq) {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(subType);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint(endpoint);
        environment.setPort(port);
        return check(resource, environment, checkAppReq);
    }

    @Routing(destinationIp = "#{agent.endpoint}")
    @Override
    public AgentBaseDto checkNoRetry(ProtectedResource resource, ProtectedEnvironment agent, CheckAppReq checkAppReq) {
        return check(resource, agent, checkAppReq, false);
    }

    private AgentBaseDto check(ProtectedResource resource, ProtectedEnvironment agent, CheckAppReq checkAppReq,
        boolean isRetry) {
        URI requestUri = RequestUriUtil.getRequestUri(agent.getEndpoint(), agent.getPort());
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        try {
            log.info("Begin to checkApplication, resource(UUID: {}, name: {}), agent(id: {}, name: {},"
                    + "uri: {}, subType:{}).", resource.getUuid(), resource.getName(), agent.getUuid(), agent.getName(),
                requestUri, resource.getSubType());
            agentBaseDto = (isRetry ? getAgentUnifiedRestApi(agent.getEndpoint())
                    : getNoRetryAgentUnifiedRestApi(agent.getEndpoint())).check(requestUri,
                resource.getSubType(), checkAppReq);
        } catch (LegoCheckedException e) {
            log.error("Fail to check application, uri: {}.", requestUri, e);
            agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            if (VerifyUtil.isEmpty(e.getMessage())) {
                log.info("Check failed, error msg is empty, uuid: {}.", resource.getUuid());
                ActionResult actionResult = new ActionResult();
                actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
                actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
                agentBaseDto.setErrorMessage(JSONObject.writeValueAsString(actionResult));
            } else {
                agentBaseDto.setErrorMessage(e.getMessage());
            }
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Fail to check application, call agent api failed, uri: {}.", requestUri, e);
            agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            ActionResult actionResult = new ActionResult();
            actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            actionResult.setMessage(e.getMessage());
            agentBaseDto.setErrorMessage(JSONObject.writeValueAsString(actionResult));
        }
        log.info("Check finish, agentId: {}, uri: {}, code: {}, msg: {}", agent.getUuid(), requestUri,
            agentBaseDto.getErrorCode(), agentBaseDto.getErrorMessage());
        return agentBaseDto;
    }

    @Routing(destinationIp = "#{environment.endpoint}")
    @Override
    public AppEnvResponse getClusterInfo(ProtectedResource resource, ProtectedEnvironment environment) {
        return getClusterInfo(resource, environment, true);
    }

    @Routing(destinationIp = "#{environment.endpoint}")
    @Override
    public AppEnvResponse getClusterInfo(String subType, ProtectedEnvironment environment, CheckAppReq checkAppReq,
        boolean isRetry) {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(subType);
        URI requestUri = RequestUriUtil.getRequestUri(environment.getEndpoint(), environment.getPort());
        try {
            return (isRetry ? getAgentUnifiedRestApi(environment.getEndpoint())
                    : getNoRetryAgentUnifiedRestApi(environment.getEndpoint())).queryCluster(requestUri,
                resource.getSubType(), checkAppReq);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", requestUri, e);
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    @Routing(destinationIp = "#{environment.endpoint}")
    @Override
    public AppEnvResponse getClusterInfoNoRetry(ProtectedResource resource, ProtectedEnvironment environment) {
        return getClusterInfo(resource, environment, false);
    }

    private AppEnvResponse getClusterInfo(ProtectedResource resource, ProtectedEnvironment environment,
        boolean isRetry) {
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(BeanTools.copy(resource, AppEnv::new));
        checkAppReq.setApplication(BeanTools.copy(resource, Application::new));

        URI requestUri = RequestUriUtil.getRequestUri(environment.getEndpoint(), environment.getPort());
        try {
            return (isRetry ? getAgentUnifiedRestApi(environment.getEndpoint())
                    : getNoRetryAgentUnifiedRestApi(environment.getEndpoint())).queryCluster(requestUri,
                resource.getSubType(), checkAppReq);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", requestUri, e);
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public Map<String, Object> queryAppConf(String appType, String script, String endpoint, Integer port) {
        URI requestUri = RequestUriUtil.getRequestUri(endpoint, port);
        try {
            return getAgentUnifiedRestApi(endpoint).queryAppConfig(requestUri, appType, script);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", requestUri, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public void deliverTaskStatus(String appType, DeliverTaskStatusDto deliverTaskStatusDto, String endpoint,
        Integer port) {
        URI requestUri = RequestUriUtil.getRequestUri(endpoint, port);
        try {
            getAgentUnifiedRestApi(endpoint).deliverTaskStatus(requestUri, appType, deliverTaskStatusDto);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", requestUri, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public CollectAgentLogRsp collectAgentLog(String endpoint, Integer port) {
        try {
            URI requestUri = RequestUriUtil.getRequestUri(endpoint, port);
            log.debug("collect agent log, uri: {}", requestUri);
            return getAgentUnifiedRestApi(endpoint).collectAgentLog(requestUri);
        } catch (LegoCheckedException e) {
            log.error("collect agent log error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (FeignException e) {
            log.error("collect agent log error feign error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public GetAgentLogCollectStatusRsp getCollectAgentLogStatus(String endpoint, Integer port) {
        try {
            URI requestUri = RequestUriUtil.getRequestUri(endpoint, port);
            log.debug("get collect agent log status, uri: {}", requestUri);
            return getAgentUnifiedRestApi(endpoint).getCollectAgentLogStatus(requestUri);
        } catch (LegoCheckedException e) {
            log.error("get collect agent log status error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (FeignException e) {
            log.error("get collect agent log status feign error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public Response exportAgentLog(String endpoint, Integer port, String id, long maxSize) {
        try {
            URI requestUri = RequestUriUtil.getRequestUri(endpoint, port);
            log.debug("export agent log, uri: {}, id: {}, maxSize: {}", requestUri, id, maxSize);
            return getAgentUnifiedRestApi(endpoint).exportAgentLog(requestUri, id, maxSize);
        } catch (LegoCheckedException e) {
            log.error("export agent log error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (FeignException e) {
            log.error("export agent log feign error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public void updateAgentLogLevel(String endpoint, Integer port, UpdateAgentLevelReq configReq) {
        try {
            URI requestUri = RequestUriUtil.getRequestUri(endpoint, port);
            log.debug("update agent config, uri: {}", requestUri);
            getAgentUnifiedRestApi(endpoint).updateAgentLogLevel(requestUri, configReq);
        } catch (LegoCheckedException e) {
            log.error("update agent config error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (FeignException e) {
            log.error("update agent config feign error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public void cleanAgentLog(String endpoint, Integer port, CleanAgentLogReq cleanAgentLogReq) {
        try {
            URI requestUri = RequestUriUtil.getRequestUri(endpoint, port);
            log.debug("clean agent log, uri: {}", requestUri);
            getAgentUnifiedRestApi(endpoint).cleanAgentLog(requestUri, cleanAgentLogReq);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("clean agent log error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public String queryRelatedClusterEsnByAgent(String endpoint, Integer port) {
        try {
            URI requestUri = RequestUriUtil.getRequestUri(endpoint, port);
            GetClusterEsnReq clusterEsn = getAgentUnifiedRestApi(endpoint).getClusterEsn(requestUri);
            log.debug("get cluster esn by agent, uri: {}, esn: {}", requestUri, clusterEsn);
            if (VerifyUtil.isEmpty(clusterEsn)) {
                log.debug("get cluster esn by agent, esn: {}", clusterEsn);
                return "";
            }
            return clusterEsn.getEsn();
        } catch (LegoCheckedException e) {
            log.error("get cluster esn by agent error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (FeignException e) {
            log.error("get cluster esn by agent feign error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public void removeProtectUnmountRepo(String endpoint, Integer port, String appType, String reqBody) {
        removeProtectUnmountRepo(endpoint, port, appType, reqBody, true);
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public void removeProtectUnmountRepoNoRetry(String endpoint, Integer port, String appType, String reqBody) {
        removeProtectUnmountRepo(endpoint, port, appType, reqBody, false);
    }

    private void removeProtectUnmountRepo(String endpoint, Integer port, String appType, String reqBody,
        boolean isRetry) {
        try {
            URI requestUri = RequestUriUtil.getRequestUri(endpoint, port);
            log.info("remote protect unmount repository, uri: {}", requestUri);
            (isRetry ? getAgentUnifiedRestApi(endpoint)
                    : getNoRetryAgentUnifiedRestApi(endpoint)).removeProtectUnmountRepo(requestUri,
                appType, reqBody);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("remote protect unmount repository error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    /**
     * 查询agent的wwpn
     *
     * @param endpoint agent上报环境信息的endpoint
     * @param port agent上报环境信息的port
     * @param version 调用新旧接口查询wwpn v1:通用代理查询wwpn v2:SanClient和AIX主机查询wwpn
     * @return agent返回的主机信息
     */
    @Routing(destinationIp = "#{endpoint}")
    @Override
    public AgentWwpnInfo listWwpn(String endpoint, Integer port, String version) {
        URI uri = RequestUriUtil.getRequestUri(endpoint, port);
        log.debug("Common host environment, get agent host, uri: {}", uri);
        try {
            AgentUnifiedRestApi agentRestApi = getAgentUnifiedRestApi(endpoint);
            return (StringUtils.isBlank(version) || StringUtils.equalsIgnoreCase("v1", version))
                ? agentRestApi.listWwpn(uri)
                : agentRestApi.listWwpnV2(uri);
        } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", uri, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    /**
     * 获取iqn验证信息
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @param agentIqnValidateRequest agentIqnValidateRequest
     * @return AgentWwpnInfo agentWwpnInfo
     */
    @Routing(destinationIp = "#{endpoint}")
    @Override
    public AgentWwpnInfo getIqnInfoList(String endpoint, Integer port,
        AgentIqnValidateRequest agentIqnValidateRequest) {
        URI uri = RequestUriUtil.getRequestUri(endpoint, port);
        log.debug("getIqnInfoList, get agent host, uri: {}", uri);
        try {
            return getAgentUnifiedRestApi(endpoint).getIqnInfoList(uri, agentIqnValidateRequest);
        } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", uri, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    /**
     * 扫描SanClient IQN信息 一个获取SanClient主机最多只有一个IQN
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @return AgentWwpnInfo agentWwpnInfo
     */
    @Routing(destinationIp = "#{endpoint}")
    @Override
    public AgentWwpnInfo scanSanClientIqnInfo(String endpoint, Integer port) {
        URI uri = RequestUriUtil.getRequestUri(endpoint, port);
        log.debug("Start to get ClientIqnInfo, get agent host, uri: {}", uri);
        try {
            return getAgentUnifiedRestApi(endpoint).scanSanClientIqnInfo(uri);
        } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", uri, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    /**
     * 查询agent的wwpn
     *
     * @param endpoint agent上报环境信息的endpoint
     * @param port agent上报环境信息的port
     * @return agent返回的主机信息
     */
    @Routing(destinationIp = "#{endpoint}")
    @Override
    public AgentBaseDto reScan(String endpoint, Integer port) {
        URI uri = RequestUriUtil.getRequestUri(endpoint, port);
        log.debug("Common host environment, get agent host, uri: {}", uri);
        try {
            return getAgentUnifiedRestApi(endpoint).reScan(uri);
        } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", uri, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    private PageListResponse<ProtectedResource> convertToPageList(ListResourceV2Rsp listResourceV2Rsp) {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>(0, new ArrayList<>());

        // 如果没报错，但无数据，则也当做返回空结构体
        if (listResourceV2Rsp == null) {
            return pageListResponse;
        }

        // 返回错误码，抛出错误码
        if (Objects.nonNull(listResourceV2Rsp.getErrorCode()) && !SUCCESS_ERROR_CODE.equals(
            listResourceV2Rsp.getErrorCode())) {
            log.info("get details error, errorCode: {}, errorMessage: {}", listResourceV2Rsp.getErrorCode(),
                listResourceV2Rsp.getErrorMessage());
            throw new LegoCheckedException(Long.parseLong(listResourceV2Rsp.getErrorCode()),
                listResourceV2Rsp.getErrorMessage());
        }

        // 列表为空无数据，返回空结构体
        if (listResourceV2Rsp.getResourceListDto() == null) {
            return pageListResponse;
        }

        // 有数据
        ResourceListDto resourceListDto = listResourceV2Rsp.getResourceListDto();
        pageListResponse.setTotalCount(resourceListDto.getTotal());
        if (resourceListDto.getItems() == null) {
            return pageListResponse;
        }
        List<AppResource> appResources = resourceListDto.getItems();
        List<ProtectedResource> records = appResources.stream()
            .map(appResource -> BeanTools.copy(appResource, ProtectedResource::new))
            .collect(Collectors.toList());
        pageListResponse.setRecords(records);
        return pageListResponse;
    }

    private Proxy getDmeProxy(String endPoint) {
        Optional<Proxy> proxyOptional = domainService.getDmeProxy(endPoint);
        return proxyOptional.get();
    }

    /**
     * 将agent返回的plugin对象，转换为ProtectedResource对象
     *
     * @param pluginsDto agent返回的plugin对象
     * @return 受保护对象列表
     */
    private List<ProtectedResource> convertPluginsToProtectedResources(PluginsDto pluginsDto) {
        // 主机uuid
        String uuid = pluginsDto.getUuid();
        if (VerifyUtil.isEmpty(uuid)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Uuid can not be empty.");
        }

        List<SupportPluginDto> plugins = pluginsDto.getSupportPlugins();
        log.debug("Host environment scan, agent support plugins: {}", plugins);
        if (VerifyUtil.isEmpty(plugins)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Plugins are empty");
        }
        List<ProtectedResource> protectedResources = new ArrayList<>();
        for (SupportPluginDto plugin : plugins) {
            String pluginName = plugin.getPluginName();
            String pluginVersion = plugin.getPluginVersion();
            List<SupportApplicationDto> supportApplications = plugin.getSupportApplications();
            if (supportApplications == null) {
                continue;
            }
            for (SupportApplicationDto supportApplication : supportApplications) {
                ProtectedResource protectedResource = new ProtectedResource();
                Map<String, String> extendInfo = new HashMap<>();
                String subType = supportApplication.getApplication();
                String pluginUuid = genPluginUuid(uuid, subType);

                protectedResource.setUuid(pluginUuid);
                protectedResource.setParentUuid(uuid);
                protectedResource.setRootUuid(uuid);
                protectedResource.setType(PLUGIN_RESOURCE_TYPE);
                protectedResource.setSubType(subType + PLUGIN_RESOURCE_TYPE);
                protectedResource.setName(pluginName);
                extendInfo.put("pluginVersion", pluginVersion);
                if (!VerifyUtil.isEmpty(supportApplication.getMaxVersion())) {
                    extendInfo.put("appMaxVersion", supportApplication.getMaxVersion());
                }
                if (!VerifyUtil.isEmpty(supportApplication.getMinVersion())) {
                    extendInfo.put("appMinVersion", supportApplication.getMinVersion());
                }
                protectedResource.setExtendInfo(extendInfo);

                protectedResources.add(protectedResource);
            }
        }

        return protectedResources;
    }

    private String genPluginUuid(String uuid, String subType) {
        String pluginIdentity = uuid + subType;

        return UUID.nameUUIDFromBytes(pluginIdentity.getBytes(Charset.defaultCharset())).toString();
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public AgentUpdateResponse modifyPluginType(String endpoint, Integer port, UpdateAgentPluginTypeReq req) {
        try {
            URI requestUri = RequestUriUtil.getRequestUri(endpoint, port);
            log.info("modify agent plugin type, uri: {}", requestUri);
            return getAgentUnifiedRestApi(endpoint).modifyPlugin(requestUri, req);
        } catch (LegoCheckedException e) {
            log.error("modify agent plugin type fail, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (FeignException e) {
            log.error("modify agent plugin type error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    @Routing(destinationIp = "#{endpoint}")
    @Override
    public AgentUpdatePluginTypeResult getModifyPluginTypeResult(String endpoint, Integer port) {
        try {
            URI requestUri = RequestUriUtil.getRequestUri(endpoint, port);
            log.info("modify agent plugin type, uri: {}", requestUri);
            return getAgentUnifiedRestApi(endpoint).getModifyPluginTypeResult(requestUri);
        } catch (LegoCheckedException e) {
            log.error("modify agent plugin type fail, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (FeignException e) {
            log.error("modify agent plugin type error, endpoint: {}, port: {}", endpoint, port,
                ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    /**
     * 推送证书给agent
     *
     * @param agent agent环境信息
     * @param pushUpdateCertToAgentReq 封装证书的参数
     * @return AgentBaseDto
     */
    @Routing(destinationIp = "#{agent.endpoint}")
    @Override
    public AgentBaseDto pushCertToAgent(ProtectedEnvironment agent, PushUpdateCertToAgentReq pushUpdateCertToAgentReq) {
        URI requestUri = RequestUriUtil.getRequestUri(agent.getEndpoint(), agent.getPort());
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        try {
            log.info("Begin to push file to agent, agent id is: {}.", agent.getUuid());
            agentBaseDto = getAgentUnifiedRestApi(agent.getEndpoint()).pushCertToAgent(requestUri,
                    pushUpdateCertToAgentReq);
        } catch (LegoCheckedException e) {
            log.error("Push file to agent failed, agent id is: {}.", agent.getUuid(), ExceptionUtil.getErrorMessage(e));
            agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            if (VerifyUtil.isEmpty(e.getMessage())) {
                ActionResult actionResult = new ActionResult();
                actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
                actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
                agentBaseDto.setErrorMessage(JSONObject.writeValueAsString(actionResult));
            } else {
                agentBaseDto.setErrorMessage(e.getMessage());
            }
        } catch (LegoUncheckedException | FeignException e) {
            log.info("Push file to agent failed, agent id is: {}.", agent.getUuid(), ExceptionUtil.getErrorMessage(e));
            agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            ActionResult actionResult = new ActionResult();
            actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            actionResult.setMessage(e.getMessage());
            agentBaseDto.setErrorMessage(JSONObject.writeValueAsString(actionResult));
        }
        return agentBaseDto;
    }

    /**
     * 通知agent更新证书
     *
     * @param agent agent环境信息
     * @param pushUpdateCertToAgentReq 封装证书的参数
     * @return AgentBaseDto
     */
    @Routing(destinationIp = "#{agent.endpoint}")
    @Override
    public AgentBaseDto notifyAgentUpdateCert(ProtectedEnvironment agent,
        PushUpdateCertToAgentReq pushUpdateCertToAgentReq) {
        URI requestUri = RequestUriUtil.getRequestUri(agent.getEndpoint(), agent.getPort());
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        try {
            log.info("Begin to notify agent to update file, agent id is: {}.", agent.getUuid());
            agentBaseDto = getAgentUnifiedRestApi(agent.getEndpoint()).notifyAgentUpdateCert(requestUri,
                    pushUpdateCertToAgentReq);
        } catch (LegoCheckedException e) {
            log.error("Notify agent to update file failed, agent id is: {}.", agent.getUuid(),
                ExceptionUtil.getErrorMessage(e));
            agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            if (VerifyUtil.isEmpty(e.getMessage())) {
                ActionResult actionResult = new ActionResult();
                actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
                actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
                agentBaseDto.setErrorMessage(JSONObject.writeValueAsString(actionResult));
            } else {
                agentBaseDto.setErrorMessage(e.getMessage());
            }
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Notify agent to update file failed, agent id is: {}.", agent.getUuid(),
                ExceptionUtil.getErrorMessage(e));
            agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            ActionResult actionResult = new ActionResult();
            actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            actionResult.setMessage(e.getMessage());
            agentBaseDto.setErrorMessage(JSONObject.writeValueAsString(actionResult));
        }
        return agentBaseDto;
    }

    /**
     * 通知agent回退证书
     *
     * @param agent agent环境信息
     * @param pushUpdateCertToAgentReq 封装证书的参数
     * @return AgentBaseDto
     */
    @Routing(destinationIp = "#{agent.endpoint}")
    @Override
    public AgentBaseDto notifyAgentFallbackCert(ProtectedEnvironment agent,
        PushUpdateCertToAgentReq pushUpdateCertToAgentReq) {
        URI requestUri = RequestUriUtil.getRequestUri(agent.getEndpoint(), agent.getPort());
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        String jobId = pushUpdateCertToAgentReq.getJobId();
        try {
            log.info("Begin to notify agent to fallback file, agentId: {}, jobId: {}.", agent.getUuid(), jobId);
            agentBaseDto = getAgentUnifiedRestApi(agent.getEndpoint()).notifyAgentFallbackCert(requestUri,
                    pushUpdateCertToAgentReq);
        } catch (LegoCheckedException e) {
            log.error("Notify agent to fallback file failed, agentId: {}, jobId: {}.", agent.getUuid(), jobId,
                ExceptionUtil.getErrorMessage(e));
            agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            if (VerifyUtil.isEmpty(e.getMessage())) {
                ActionResult actionResult = new ActionResult();
                actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
                actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
                agentBaseDto.setErrorMessage(JSONObject.writeValueAsString(actionResult));
            } else {
                agentBaseDto.setErrorMessage(e.getMessage());
            }
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Notify agent to fallback file failed, agentId: {}, jobId: {}.", agent.getUuid(), jobId,
                ExceptionUtil.getErrorMessage(e));
            agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            ActionResult actionResult = new ActionResult();
            actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            actionResult.setMessage(e.getMessage());
            agentBaseDto.setErrorMessage(JSONObject.writeValueAsString(actionResult));
        }
        return agentBaseDto;
    }

    /**
     * 通知agent删除旧证书
     *
     * @param agent agent环境信息
     * @param pushUpdateCertToAgentReq 封装证书的参数
     * @return AgentBaseDto
     */
    @Routing(destinationIp = "#{agent.endpoint}")
    @Override
    public AgentBaseDto notifyAgentDeleteOldCert(ProtectedEnvironment agent,
        PushUpdateCertToAgentReq pushUpdateCertToAgentReq) {
        URI requestUri = RequestUriUtil.getRequestUri(agent.getEndpoint(), agent.getPort());
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        String jobId = pushUpdateCertToAgentReq.getJobId();
        try {
            log.info("Begin to notify agent to delete old file, agentId: {}, jobId: {}.", agent.getUuid(), jobId);
            agentBaseDto = getAgentUnifiedRestApi(agent.getEndpoint()).notifyAgentDeleteOldCert(requestUri,
                    pushUpdateCertToAgentReq);
        } catch (LegoCheckedException e) {
            log.info("Notify agent to delete old file failed, agentId: {}, jobId: {}.", agent.getUuid(), jobId,
                ExceptionUtil.getErrorMessage(e));
            agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            if (VerifyUtil.isEmpty(e.getMessage())) {
                ActionResult actionResult = new ActionResult();
                actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
                actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
                agentBaseDto.setErrorMessage(JSONObject.writeValueAsString(actionResult));
            } else {
                agentBaseDto.setErrorMessage(e.getMessage());
            }
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Notify agent to delete old file failed, agentId: {}, jobId: {}.", agent.getUuid(), jobId,
                ExceptionUtil.getErrorMessage(e), ExceptionUtil.getErrorMessage(e));
            agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            ActionResult actionResult = new ActionResult();
            actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            actionResult.setMessage(e.getMessage());
            agentBaseDto.setErrorMessage(JSONObject.writeValueAsString(actionResult));
        }
        return agentBaseDto;
    }

    /**
     * pm检查与agent的连通性
     *
     * @param agent agent环境信息
     * @return AgentBaseDto
     */
    @Routing(destinationIp = "#{agent.endpoint}")
    @Override
    public AgentBaseDto checkConnection(ProtectedEnvironment agent) {
        URI requestUri = RequestUriUtil.getRequestUri(agent.getEndpoint(), agent.getPort());
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        try {
            log.info("Begin to check connection to agent, agent id is: {}.", agent.getUuid());
            agentBaseDto = getAgentUnifiedRestApi(agent.getEndpoint()).checkConnection(requestUri);
        } catch (LegoCheckedException e) {
            log.info("Check connection to agent failed, agent id is: {}.", agent.getUuid(),
                ExceptionUtil.getErrorMessage(e));
            agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            if (VerifyUtil.isEmpty(e.getMessage())) {
                ActionResult actionResult = new ActionResult();
                actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
                actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
                agentBaseDto.setErrorMessage(JSONObject.writeValueAsString(actionResult));
            } else {
                agentBaseDto.setErrorMessage(e.getMessage());
            }
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Check connection to agent failed, agent id is: {}.", agent.getUuid(),
                ExceptionUtil.getErrorMessage(e), ExceptionUtil.getErrorMessage(e));
            agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            ActionResult actionResult = new ActionResult();
            actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            actionResult.setMessage(e.getMessage());
            agentBaseDto.setErrorMessage(JSONObject.writeValueAsString(actionResult));
        }
        return agentBaseDto;
    }

    @Routing(destinationIp = "#{agent.endpoint}")
    @Override
    public void updateAgentServer(ProtectedEnvironment agent, ManagementIp managementIp) {
        URI requestUri = RequestUriUtil.getRequestUri(agent.getEndpoint(), agent.getPort());
        try {
            getAgentUnifiedRestApi(agent.getEndpoint()).modifyAgentServerList(requestUri, managementIp);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Call agent api failed, uri: {}", requestUri, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }
}
