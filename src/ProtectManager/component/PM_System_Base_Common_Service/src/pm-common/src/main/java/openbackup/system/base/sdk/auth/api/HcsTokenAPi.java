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
package openbackup.system.base.sdk.auth.api;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.HcsConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.auth.model.HcsProjectsPageResponse;
import openbackup.system.base.sdk.auth.model.HcsToken;
import openbackup.system.base.sdk.auth.model.IpQueryRequest;
import openbackup.system.base.sdk.auth.model.QueryCloudServiceParamsPageResponse;
import openbackup.system.base.sdk.auth.model.QueryIpResponse;
import openbackup.system.base.sdk.auth.model.QueryPasswordResponse;
import openbackup.system.base.sdk.auth.model.URIInfo;
import openbackup.system.base.sdk.auth.model.request.CloudServiceQueryRequest;
import openbackup.system.base.util.OpServiceUtil;

import feign.Response;

import org.apache.commons.lang3.StringUtils;

import java.net.URI;
import java.net.URISyntaxException;

/**
 * 调用Hcs平台的Api
 *
 * @author l30044826
 * @since 2023-07-12
 */
public interface HcsTokenAPi {
    /**
     * 根据项目id获取token信息
     *
     * @param projectId 项目id，为null时，使用domain域
     * @return HcsToken
     */
    HcsToken getTokenByProjectId(String projectId);

    /**
     * IAM 校验hcs token是否正确，正确则解析
     *
     * @param token 使用的token
     * @param subjectToken 待校验的token
     * @return HcsToken
     */
    HcsToken verifyAuthToken(String token, String subjectToken);

    /**
     * IAM 校验hcs token是否正确，正确则解析
     *
     * @param token 使用的token
     * @param subjectToken 待校验的token
     * @param globalDomainName hcs对外全局域名
     * @return HcsToken
     */
    HcsToken verifyAuthToken(String token, String subjectToken, String globalDomainName);

    /**
     * 根据条件查询云服务参数信息
     *
     * @param uri uri
     * @param token hcs用户token
     * @param paramName 参数名称
     * @param cloudServiceQueryRequest 云服务查询参数
     * @return 查询响应
     */
    QueryCloudServiceParamsPageResponse queryCloudServiceParams(URI uri, String token, String paramName,
        CloudServiceQueryRequest cloudServiceQueryRequest);

    /**
     * 十统一根据region和用户名查询 统一管理sysadmin密码，不用代理
     *
     * @param uri uri
     * @param baseQueryCondition 查询参数
     * @param hcsToken 运营面管理员的token
     * @return QueryPasswordResponse
     */
    QueryPasswordResponse queryPasswordInfoNoProxy(URI uri, String baseQueryCondition, String hcsToken);

    /**
     * 查询用户下全部资源集信息（与Hcs ManageOne查询条件保持一致）
     *
     * @param uri uri
     * @param token hcs用户token
     * @param userId 用户id
     * @param projectId projectId
     * @return 查询响应
     */
    HcsProjectsPageResponse queryUserProjects(URI uri, String token, String userId, String projectId);

    /**
     * 查询用户下全部资源集信息（与Hcs ManageOne查询条件保持一致），不用代理
     *
     * @param uri uri
     * @param token hcs用户token
     * @param userId 用户id
     * @return 查询响应
     */
    HcsProjectsPageResponse queryUserProjectsNoProxy(URI uri, String token, String userId);

    /**
     * 根据接口类型以及名称获取url
     *
     * @param token token
     * @param interfaceType interfaceType
     * @param name name
     * @return URIInfo
     */
    URIInfo getUrlByInterfaceTypeAndName(HcsToken token, String interfaceType, String name);

    /**
     * 查询指定project信息
     *
     * @param uri uri
     * @param token hcs用户token
     * @param projectId project id
     * @return 返回体
     */
    Response getResourceSetDetailByProjectId(URI uri, String token, String projectId);

    /**
     * 根据资源集id获取domainId
     *
     * @param projectId 资源集id
     * @return 返回domainId
     */
    String getDomainIdByProjectId(String projectId);

    /**
     * 根据projectId获取租户token
     *
     * @param projectId 资源集id
     * @return 租户token
     */
    HcsToken getTenantTokenByProjectId(String projectId);

    /**
     * 获取IAM服务的地址
     *
     * @return 服务地址
     */
    default URI getHcsAuthUri() {
        URI uri = null;
        try {
            String regionId = OpServiceUtil.getRegionId();
            String globalDomainName = OpServiceUtil.getGlobalDomainName();
            if (VerifyUtil.isEmpty(regionId) || VerifyUtil.isEmpty(globalDomainName)) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "hcsIam get failed from env!");
            }
            String globalIamUri = StringUtils.join("https://iam-cache-proxy.", regionId, ".", globalDomainName,
                ":26335");
            uri = new URI(globalIamUri);
        } catch (URISyntaxException e) {
            throw new LegoCheckedException("uri error", e);
        }
        return uri;
    }

    /**
     * 获取IAM服务的地址
     *
     * @param globalDomainName hcs对外全局域名
     * @return 服务地址
     */
    default URI getHcsAuthUri(String globalDomainName) {
        URI uri;
        try {
            if (VerifyUtil.isEmpty(globalDomainName)) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "globalDomainName is empty!");
            }
            uri = new URI(HcsConstant.HTTPS_PRIFFIX + HcsConstant.IAM_AUTH_PRFFIX + globalDomainName);
        } catch (URISyntaxException e) {
            throw new LegoCheckedException("uri error", e);
        }
        return uri;
    }

    /**
     * 获取账户名
     *
     * @return 账户名
     */
    String getKey();

    /**
     * 获取密码
     *
     * @return 密码
     */
    String getAuthPwd();

    /**
     * 根据条件查询云服务参数信息，不用代理
     *
     * @param uri uri
     * @param token hcs用户token
     * @param paramName 参数名称
     * @param cloudServiceQueryRequest 云服务查询参数
     * @return 查询响应
     */
    QueryCloudServiceParamsPageResponse queryCloudServiceParamsNoProxy(URI uri, String token, String paramName,
        CloudServiceQueryRequest cloudServiceQueryRequest);

    /**
     * queryIpParams
     *
     * @param uri uri
     * @param token token
     * @param ipQueryRequest ipQueryRequest
     * @return QueryIpResponse
     */
    QueryIpResponse queryIpParams(URI uri, String token, IpQueryRequest ipQueryRequest);

    /**
     * 十统一根据region和用户名查询 统一管理sysadmin密码
     *
     * @param uri uri
     * @param basePasswds basePasswds
     * @return QueryPasswordResponse
     */
    QueryPasswordResponse queryPasswordInfo(URI uri, String basePasswds);
}
