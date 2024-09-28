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
package openbackup.openstack.protection.access.keystone;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.constant.KeyStoneConstant;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.constant.OpenstackServiceUrls;
import openbackup.openstack.protection.access.keystone.dto.AuthTokenRequestDto;
import openbackup.openstack.protection.access.keystone.dto.EndpointDto;
import openbackup.openstack.protection.access.keystone.dto.EndpointRequestDto;
import openbackup.openstack.protection.access.keystone.dto.KeystoneHostDto;
import openbackup.openstack.protection.access.keystone.dto.ServiceDto;
import openbackup.openstack.protection.access.keystone.dto.ServiceRequestDto;
import openbackup.openstack.protection.access.keystone.util.KeyStoneHttpUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.OptionalUtil;
import openbackup.system.base.util.SpringBeanUtils;

import lombok.extern.slf4j.Slf4j;
import okhttp3.Response;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * KeyStoneService调用服务
 *
 */
@Slf4j
@Component
public class KeyStoneService {
    private final ResourceService resourceService;

    public KeyStoneService(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    /**
     * 注册到OpenStack
     *
     * @param environment 环境信息
     */
    public void registerOpenstack(ProtectedEnvironment environment) {
        KeystoneHostDto keystoneHostDto = convertKeystoneHostDto(
            environment.getEndpoint(), environment.getExtendInfo().get(KeyStoneConstant.CPS_IP));
        String authKey = environment.getAuth().getAuthKey();
        String authPwd = environment.getAuth().getAuthPwd();
        String domainName = OpenstackConstant.DEFAULT_DOMAIN;
        String token = getAuthToken(authPwd, authKey, domainName, keystoneHostDto);
        String regionId = getExistRegionId(keystoneHostDto, token);
        String serviceId = getExistServiceId(keystoneHostDto, token);
        if (StringUtils.isBlank(serviceId)) {
            serviceId = createService(keystoneHostDto, token);
        }
        if (!checkEndpointExists(serviceId, keystoneHostDto, token)) {
            createEndpoint(regionId, serviceId, keystoneHostDto, token);
        }
    }

    /**
     * 校验项目的token是否有效
     *
     * @param token 待校验的token
     * @return token对应的项目ID
     */
    public String verifyProjectToken(String token) {
        log.info("verify project start.");
        KeystoneHostDto keystoneHostDto = queryEnvKeystoneHost();
        String keystoneUrl = keystoneHostDto.getKeystoneUrl();
        Map<String, String> requestHeaders = new HashMap<>();
        requestHeaders.put(KeyStoneConstant.TOKEN_HEADER, token);
        requestHeaders.put(KeyStoneConstant.X_SUBJECT_TOKEN_HEADER, token);
        requestHeaders.put(KeyStoneConstant.HOST, keystoneHostDto.getHost());
        try (Response response = KeyStoneHttpUtil.syncGetRequestWithHeader(keystoneUrl + OpenstackServiceUrls.TOKEN,
            requestHeaders)) {
            if (Objects.isNull(response.body())) {
                throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "verify project body is null.");
            }
            JSONObject resJsonObj = JSONObject.fromObject(response.body().string());
            JSONObject tokenObj = resJsonObj.getJSONObject("token");
            JSONObject projectObj = tokenObj.getJSONObject("project");
            String projectId = projectObj.getString(OpenstackConstant.ID_KEY);
            log.info("verify success, project id is {}.", projectId);
            return projectId;
        } catch (IOException e) {
            log.error("parse response failed. keystoneUrl:{}", keystoneUrl, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "parse http response failed.");
        }
    }

    private KeystoneHostDto convertKeystoneHostDto(String keystoneUrl, String proxyIp) {
        KeystoneHostDto keystoneHostDto = new KeystoneHostDto();
        Pattern pattern = KeyStoneConstant.DOMAIN_PATTERN;
        Matcher matcher = pattern.matcher(keystoneUrl);
        if (matcher.find() && StringUtils.isNotBlank(proxyIp)) {
            String host = matcher.group(1);
            keystoneHostDto.setHost(host);
            keystoneHostDto.setKeystoneUrl(keystoneUrl.replace(host, proxyIp));
            return keystoneHostDto;
        }
        keystoneHostDto.setKeystoneUrl(keystoneUrl);
        keystoneHostDto.setHost(OpenstackConstant.EMPTY);
        return keystoneHostDto;
    }

    private String getAuthToken(String password, String name, String domainName, KeystoneHostDto keystoneHostDto) {
        String keystoneUrl = keystoneHostDto.getKeystoneUrl();
        log.info("Get authentication, keystoneUrl:{}, domainName:{}.", keystoneUrl, domainName);
        AuthTokenRequestDto authTokenRequestDto;
        if (StringUtils.isNotEmpty(domainName)) {
            authTokenRequestDto = AuthTokenRequestDto.initRequestDto(password, name, domainName);
        } else {
            authTokenRequestDto = AuthTokenRequestDto.initRequestDto(password, name);
        }
        Response response = KeyStoneHttpUtil.syncPostRequest(keystoneUrl + OpenstackServiceUrls.TOKEN,
            JSONObject.writeValueAsString(authTokenRequestDto), keystoneHostDto.getHost());
        return response.header(KeyStoneConstant.X_SUBJECT_TOKEN_HEADER);
    }

    private KeystoneHostDto queryEnvKeystoneHost() {
        Map<String, Object> filterMap = new HashMap<>();
        filterMap.put("type", ResourceTypeEnum.OPEN_STACK.getType());
        filterMap.put("subType", ResourceSubTypeEnum.OPENSTACK_CONTAINER.getType());
        PageListResponse<ProtectedResource> listResponse = resourceService.query(LegoNumberConstant.ZERO,
            LegoNumberConstant.ONE, filterMap);
        if (listResponse.getRecords().isEmpty()) {
            log.error("openstack environment not exists.");
            return new KeystoneHostDto();
        }
        return Optional.ofNullable(listResponse.getRecords().get(0))
            .flatMap(OptionalUtil.match(ProtectedEnvironment.class))
            .map(env -> convertKeystoneHostDto(
                env.getEndpoint(), env.getExtendInfo().get(KeyStoneConstant.CPS_IP)))
            .orElseGet(KeystoneHostDto::new);
    }

    private String createService(KeystoneHostDto keystoneHostDto, String token) {
        String keystoneUrl = keystoneHostDto.getKeystoneUrl();
        log.info("Create service, keystoneUrl:{}.", keystoneUrl);
        ServiceRequestDto serviceRequestDto = new ServiceRequestDto();
        ServiceDto serviceDto = new ServiceDto();
        serviceDto.setType(KeyStoneConstant.SERVICE_TYPE);
        serviceDto.setName(KeyStoneConstant.SERVICE_NAME);
        serviceDto.setDescription("The service for OceanProtect.");
        serviceRequestDto.setService(serviceDto);
        try (Response response = KeyStoneHttpUtil.syncPostRequest(keystoneUrl + OpenstackServiceUrls.SERVICE,
            JSONObject.writeValueAsString(serviceRequestDto), token, keystoneHostDto.getHost())) {
            if (Objects.isNull(response.body())) {
                throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "response body is empty.");
            }
            JSONObject resJsonObj = JSONObject.fromObject(response.body().string());
            JSONObject service = resJsonObj.getJSONObject(KeyStoneConstant.RESP_SERVICE);
            return service.getString(OpenstackConstant.ID_KEY);
        } catch (IOException e) {
            log.error("parse response failed. keystoneUrl:{}", keystoneUrl, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "parse http response failed.");
        }
    }

    private void createEndpoint(String regionId, String serviceId, KeystoneHostDto keystoneHostDto, String token) {
        String keystoneUrl = keystoneHostDto.getKeystoneUrl();
        log.info("Create endpoint, keystoneUrl:{}", keystoneUrl);
        EndpointDto endpointDto = new EndpointDto();
        endpointDto.setInterfaces(KeyStoneConstant.ENDPOINT_INTERFACE);
        endpointDto.setRegionId(regionId);
        endpointDto.setUrl("https://" + getPmIp() + ":" + KeyStoneConstant.REGISTER_PORT);
        endpointDto.setServiceId(serviceId);
        EndpointRequestDto endpointRequestDto = new EndpointRequestDto();
        endpointRequestDto.setEndpoint(endpointDto);
        KeyStoneHttpUtil.syncPostRequest(keystoneUrl + OpenstackServiceUrls.ENDPOINT,
            JSONObject.writeValueAsString(endpointRequestDto), token, keystoneHostDto.getHost());
    }

    private boolean checkEndpointExists(String serviceId, KeystoneHostDto keystoneHostDto, String token) {
        String keystoneUrl = keystoneHostDto.getKeystoneUrl();
        log.info("check endpoint exists, serviceId:{}", serviceId);
        Map<String, String> requestParam = new HashMap<>();
        requestParam.put("service_id", serviceId);
        try (Response response = KeyStoneHttpUtil.syncGetRequest(keystoneUrl + OpenstackServiceUrls.ENDPOINT,
            requestParam, token, keystoneHostDto.getHost())) {
            if (Objects.isNull(response.body())) {
                return false;
            }
            JSONObject resJsonObj = JSONObject.fromObject(response.body().string());
            JSONArray endpoints = resJsonObj.getJSONArray(KeyStoneConstant.RESP_ENDPOINTS);
            for (Object endpoint : endpoints) {
                JSONObject endpointObj = parseJsonObject(endpoint);
                String endpointUrl = "https://" + getPmIp() + ":" + KeyStoneConstant.REGISTER_PORT;
                if (endpointObj.get(KeyStoneConstant.RESP_URL).equals(endpointUrl)) {
                    log.info("endpoint exists, no need to create again.");
                    return true;
                }
            }
            return false;
        } catch (IOException e) {
            log.error("parse response failed. serviceId:{} keystoneUrl:{}", serviceId, keystoneUrl,
                ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "parse http response failed.");
        }
    }

    private JSONObject parseJsonObject(Object obj) {
        JSONObject endpointObj = new JSONObject();
        if (obj instanceof JSONObject) {
            endpointObj = (JSONObject) obj;
        }
        return endpointObj;
    }

    private String getExistRegionId(KeystoneHostDto keystoneHostDto, String token) {
        String keystoneUrl = keystoneHostDto.getKeystoneUrl();
        log.info("get exist region id, keystoneUrl:{}", keystoneUrl);
        try (Response response = KeyStoneHttpUtil.syncGetRequest(keystoneUrl + OpenstackServiceUrls.REGION,
            new HashMap<>(), token, keystoneHostDto.getHost())) {
            if (Objects.isNull(response.body())) {
                return OpenstackConstant.EMPTY;
            }
            JSONObject resJsonObj = JSONObject.fromObject(response.body().string());
            JSONArray regions = resJsonObj.getJSONArray(KeyStoneConstant.RESP_REGIONS);
            for (Object region : regions) {
                JSONObject endpointObj = parseJsonObject(region);
                String resRegionId = endpointObj.getString(KeyStoneConstant.RESP_ID);
                if (StringUtils.isNotBlank(resRegionId)) {
                    log.info("get region id success. regionId:{}", resRegionId);
                    return resRegionId;
                }
            }
            return OpenstackConstant.EMPTY;
        } catch (IOException e) {
            log.error("parse response failed. keystoneUrl:{}", keystoneUrl, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "parse http response failed.");
        }
    }

    private String getExistServiceId(KeystoneHostDto keystoneHostDto, String token) {
        String keystoneUrl = keystoneHostDto.getKeystoneUrl();
        log.info("Get exist service id, keystoneUrl:{}", keystoneUrl);
        Map<String, String> requestParam = new HashMap<>();
        requestParam.put(KeyStoneConstant.REQUEST_NAME, KeyStoneConstant.SERVICE_NAME);
        try (Response response = KeyStoneHttpUtil.syncGetRequest(keystoneUrl + OpenstackServiceUrls.SERVICE,
            requestParam, token, keystoneHostDto.getHost())) {
            if (Objects.isNull(response.body())) {
                return OpenstackConstant.EMPTY;
            }
            JSONObject resJsonObj = JSONObject.fromObject(response.body().string());
            JSONArray services = resJsonObj.getJSONArray(KeyStoneConstant.RESP_SERVICES);
            if (services.isEmpty()) {
                return OpenstackConstant.EMPTY;
            }
            log.info("service exists, no need to create again.");
            JSONObject serviceObj = parseJsonObject(services.get(0));
            return serviceObj.get(KeyStoneConstant.RESP_ID).toString();
        } catch (IOException e) {
            log.error("parse response failed. keystoneUrl:{}", keystoneUrl, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "parse http response failed.");
        }
    }

    private String getPmIp() {
        ClusterInternalApi clusterInternalApi = SpringBeanUtils.getBean(ClusterInternalApi.class);
        ClusterDetailInfo clusterDetailInfo = clusterInternalApi.queryClusterDetails();
        List<String> mgrIpList = Optional.ofNullable(clusterDetailInfo)
            .map(ClusterDetailInfo::getSourceClusters)
            .map(SourceClustersParams::getMgrIpList)
            .orElse(new ArrayList<>());
        if (mgrIpList.isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.INSTALL_CONTAINER_STATUS_ERROR, "Container status error.");
        }
        return mgrIpList.get(0);
    }
}
