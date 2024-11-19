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
package openbackup.system.base.util;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.RequestUtil;
import openbackup.system.base.common.utils.network.Ipv6AddressUtil;

import org.apache.commons.fileupload.FileUploadBase;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpHeaders;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.web.multipart.MultipartFile;
import org.springframework.web.multipart.MultipartHttpServletRequest;

import java.io.BufferedReader;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Locale;

import javax.servlet.http.HttpServletRequest;

/**
 * 外部请求处理Service
 *
 */
@Slf4j
public class RouterServiceUtils {
    private static final String HTTP_URL_SCHEME = "https://";

    private static final String EXTERNAL_CLUSTER_PREFIXED = "/v1/external";

    private RouterServiceUtils() {
    }

    /**
     * 以字符串形式获取Request中的body
     *
     * @param request httpServletRequest
     * @return String形式的body
     */
    public static Object getBodyData(HttpServletRequest request) {
        if (isMultipartRequest(request)) {
            return getFormData(request);
        }
        StringBuffer data = new StringBuffer();
        String line = null;
        try (BufferedReader reader = request.getReader()) {
            while ((line = reader.readLine()) != null) {
                data.append(line);
            }
        } catch (IOException e) {
            log.error("Fail to close IO stream.", ExceptionUtil.getErrorMessage(e));
        }
        return data.toString();
    }

    private static MultiValueMap<String, Object> getFormData(HttpServletRequest request) {
        log.info("Up to file.");
        MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
        request.getParameterMap().forEach((k, v) -> {
            if (v.length == 1) {
                map.add(k, v[0]);
            } else {
                map.add(k, v);
            }
        });
        MultipartHttpServletRequest multipartRequest;
        if (request instanceof MultipartHttpServletRequest) {
            multipartRequest = (MultipartHttpServletRequest) request;
        } else {
            return map;
        }
        Iterator<String> fileNames = multipartRequest.getFileNames();
        while (fileNames.hasNext()) {
            String fileName = fileNames.next();
            log.info("FileName:{}", fileName);
            MultipartFile file = multipartRequest.getFile(fileName);
            if (file != null) {
                Resource uploadResource = file.getResource();
                map.add(fileName, uploadResource);
            }
        }
        return map;
    }

    /**
     * 構造访问外部集群的uri
     *
     * @param httpServletRequest 请求参数
     * @param ip 集群ip
     * @param port 集群端口
     * @return 转发所需的uri
     */
    public static URI buildClusterUri(HttpServletRequest httpServletRequest, String ip, int port) {
        String clusterIp = Ipv6AddressUtil.isIpv6Address(ip) ? "[" + ip + "]" : ip;
        String clusterUri = HTTP_URL_SCHEME + clusterIp + ":" + port
                + httpServletRequest.getRequestURI().replace(EXTERNAL_CLUSTER_PREFIXED, "").replace("/v1/member", "");
        String queryString = httpServletRequest.getQueryString();
        if (queryString != null && !isMultipartRequest(httpServletRequest)) {
            clusterUri += "?" + queryString;
        }
        try {
            return new URL(NormalizerUtil.normalizeForString(clusterUri)).toURI();
        } catch (URISyntaxException | MalformedURLException e) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Fail to build the correct cluster URI.");
        }
    }

    /**
     * 构造访问node的uri
     *
     * @param httpServletRequest 请求参数
     * @param ip 集群ip
     * @param port 集群端口
     * @return 转发所需的uri
     */
    public static URI buildNodeUri(HttpServletRequest httpServletRequest, String ip, int port) {
        String nodeIp = Ipv6AddressUtil.isIpv6Address(ip) ? "[" + ip + "]" : ip;
        String nodeUri = HTTP_URL_SCHEME + nodeIp + ":" + port
                + httpServletRequest.getRequestURI().replace("/v1/try-all-node", "");
        String queryString = httpServletRequest.getQueryString();
        if (queryString != null && !isMultipartRequest(httpServletRequest)) {
            nodeUri += "?" + queryString;
        }
        try {
            return new URL(NormalizerUtil.normalizeForString(nodeUri)).toURI();
        } catch (URISyntaxException | MalformedURLException e) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Fail to build the correct node URI.");
        }
    }

    /**
     * 构造转发请求的头部
     *
     * @param httpServletRequest 请求参数
     * @param token 集群token
     * @return 转发请求头部
     */
    public static HttpHeaders buildClusterHeader(HttpServletRequest httpServletRequest, String token) {
        HttpHeaders headers = new HttpHeaders();
        Enumeration<String> headerNameEnumeration = httpServletRequest.getHeaderNames();
        while (headerNameEnumeration.hasMoreElements()) {
            String headerName = headerNameEnumeration.nextElement();
            if ("clusters-type".equals(headerName)) {
                headers.set(headerName, "1");
                continue;
            }
            if ("x-auth-token".equals(headerName)) {
                headers.set(headerName, token);
                continue;
            }
            Enumeration<String> headerValueEnumeration = httpServletRequest.getHeaders(headerName);
            while (headerValueEnumeration.hasMoreElements()) {
                headers.add(headerName, headerValueEnumeration.nextElement());
            }
        }
        String clientIp = RequestUtil.getClientIpAddress(httpServletRequest);
        headers.add(RequestUtil.CLIENT_IP, clientIp);
        return headers;
    }

    private static boolean isMultipartRequest(HttpServletRequest request) {
        String contentType = request.getContentType();
        return contentType != null && contentType.toLowerCase(Locale.ENGLISH).contains(FileUploadBase.MULTIPART);
    }
}
