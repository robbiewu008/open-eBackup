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
package openbackup.system.base.service;

import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.ProtocolPortConstant;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.cluster.NodeRestApi;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.util.Base64Util;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.multipart.MultipartFile;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.text.Normalizer;
import java.util.ArrayList;
import java.util.List;

/**
 * k8s集群节点间通信服务
 *
 */
@Service
@Slf4j
public class KubernatesService {
    @Autowired
    private InfrastructureRestApi infrastructureRestApi;

    @Autowired
    private NodeRestApi nodeRestApi;

    /**
     * 同步multipartFile到endPointName指定的所有节点
     *
     * @param multipartFile filePath对应的文件的multipartFile
     * @param filePath filePath 带文件名的文件路径
     * @param endPointName endPointName
     */
    public void syncAlarmDumpFile(MultipartFile multipartFile, String filePath,
        String endPointName) {
        List<String> ipList = getIpListByEndPointName(endPointName);
        for (String ip : ipList) {
            try {
                String nodeUri = Constants.HTTP_URL_SCHEME + ip + ":" + getPortByEndPointName(endPointName);
                log.info("Current uri is: {}.", nodeUri);
                URI uri = new URL(normalizeForString(nodeUri)).toURI();
                String encryptToBase64 = Base64Util.encryptToBase64(filePath);
                nodeRestApi.syncAlarmDumpFile(uri, multipartFile, encryptToBase64);
            } catch (URISyntaxException | MalformedURLException e) {
                log.error("Build uri failed.", ExceptionUtil.getErrorMessage(e));
            } catch (FeignException exception) {
                log.error("Sync alarm dump file failed.", ExceptionUtil.getErrorMessage(exception));
            }
        }
    }

    /**
     * 根据endPointName获取ip列表
     *
     * @param endPointName endPointName
     * @return ip列表
     */
    private List<String> getIpListByEndPointName(String endPointName) {
        try {
            InfraResponseWithError<List<String>> endpointsResponse =
                infrastructureRestApi.getEndpoints(endPointName);
            List<String> endpoints = endpointsResponse.getData();
            if (endpoints.isEmpty()) {
                log.warn("Get all node endpoint failed.");
            }
            return endpoints;
        } catch (FeignException e) {
            log.error("Get all node endpoint failed", ExceptionUtil.getErrorMessage(e));
            return new ArrayList<>();
        }
    }

    // 过滤不安全的特殊字符
    private String normalizeForString(String item) {
        if (VerifyUtil.isEmpty(item)) {
            return "";
        }
        return Normalizer.normalize(item, Normalizer.Form.NFKC);
    }

    // 根据endPointName获取端口号
    private int getPortByEndPointName(String endPointName) {
        switch (endPointName) {
            case Constants.PM_ENDPOINT_NAME:
                return ProtocolPortConstant.PM_INTERNAL_PORT;
            default:
                return ProtocolPortConstant.PM_INTERNAL_PORT;
        }
    }
}
