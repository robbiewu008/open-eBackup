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
package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.bean.ClusterDetailInfo;
import com.huawei.emeistor.console.bean.SourceClustersParams;
import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.contant.DeployType;
import com.huawei.emeistor.console.contant.ErrorResponse;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.util.ExceptionUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RList;
import org.redisson.api.RLock;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.util.StreamUtils;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.HttpServerErrorException;
import org.springframework.web.client.RequestCallback;
import org.springframework.web.client.RestTemplate;

import java.io.InputStream;
import java.io.OutputStream;
import java.text.MessageFormat;
import java.util.Arrays;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.concurrent.TimeUnit;

import javax.servlet.http.HttpServletResponse;

/**
 * The ExportRecordController
 *
 */
@Slf4j
@RestController
@RequestMapping(ConfigConstant.CONSOLE + "/v1/export-files")
public class ExportRecordController extends AdvBaseController {
    private static final String DOWNLOAD_FILE = "/v1/export-files/{0}/action/download";

    private static final String NODE_COUNT = "/v1/clusters/details";

    /**
     * 读取锁
     */
    private static final String READ_LOCK_PREFIX = "read_export_file_list";

    private static final int DEFAULT_DOWNLOAD_THREAD_LIMIT = 5;

    private static final int PER_CONTROLLER_THREAD_LIMIT = 5;

    private static final String DELIMITER = ",";

    @Value("${api.gateway.endpoint}")
    private String sysbackupApi;

    @Value("${api.gateway.endpoint}")
    private String clusterApi;

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private RequestUtil requestUtil;

    @Autowired
    private RedissonClient redissonClient;

    /**
     * 下载导出文件
     *
     * @param id ID
     * @param subId 子记录
     * @param response 响应体
     * @return 错误场景
     */
    @ExterAttack
    @GetMapping("/{id}/action/download")
    public ResponseEntity<ErrorResponse> downloadExportFile(
        @PathVariable(name = "id") String id,
        @RequestParam(value = "subId", required = false) String subId,
        HttpServletResponse response) {
        int downloadThreadLimit = getDownloadThreadLimit();
        RLock lock = redissonClient.getLock(READ_LOCK_PREFIX + "_" + id);
        lock.lock(1, TimeUnit.SECONDS);
        RList<Object> readFileIdList = redissonClient.getList(READ_LOCK_PREFIX);
        try {
            readFileIdList.add(id);
            log.info("There are readFileIdList: {} downloads.", readFileIdList.size());
            if (readFileIdList.size() > downloadThreadLimit) {
                lock.unlock();
                ErrorResponse errorResponse = new ErrorResponse();
                errorResponse.setErrorCode(String.valueOf(CommonErrorCode.DOWNLOAD_MAX));
                errorResponse.setErrorMessage("The download threads meet limit " + downloadThreadLimit);
                errorResponse.setDetailParams(new String[] {String.valueOf(downloadThreadLimit)});
                return new ResponseEntity<>(errorResponse, HttpStatus.INTERNAL_SERVER_ERROR);
            }
            lock.unlock();
            String url = sysbackupApi + MessageFormat.format(DOWNLOAD_FILE, id);
            if (StringUtils.isNotBlank(subId)) {
                url = url + "?subId=" + subId;
            }
            restTemplate.execute(super.normalizeForString(url), HttpMethod.GET, generateRequestCallback(),
                    clientHttpResponse -> {
                try (OutputStream os = response.getOutputStream(); InputStream in = clientHttpResponse.getBody()) {
                    response.setContentType("application/x-download");
                    response.setCharacterEncoding("UTF-8");
                    response.setHeader("Pragma", "no-cache");
                    response.setHeader("Cache-Control", "no-store, must-revalidate");
                    StreamUtils.copy(in, os);
                }
                return null;
            });
        } finally {
            readFileIdList.remove(id);
        }
        return new ResponseEntity<>(HttpStatus.OK);
    }

    private RequestCallback generateRequestCallback() {
        Map<String, String> headValueMap = requestUtil.getForwardHeaderAndValidCsrf().toSingleValueMap();
        return request -> {
            request.getHeaders().setAll(headValueMap);
            request.getHeaders().setAccept(Arrays.asList(MediaType.APPLICATION_OCTET_STREAM, MediaType.ALL));
        };
    }

    private int getDownloadThreadLimit() {
        if (DeployType.getCurrentDeployType() == DeployType.CYBER_ENGINE) {
            return DEFAULT_DOWNLOAD_THREAD_LIMIT;
        }
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        HttpEntity<ClusterDetailInfo> httpEntity = new HttpEntity<>(null, headers);
        ResponseEntity<ClusterDetailInfo> responseEntity = null;
        try {
            log.info("start to getDownloadThreadLimit");
            responseEntity = restTemplate.exchange(
                super.normalizeForString(clusterApi + NODE_COUNT), HttpMethod.GET,
                httpEntity, ClusterDetailInfo.class);
        } catch (HttpServerErrorException e) {
            log.error("getDownloadThreadLimit Exception", ExceptionUtil.getErrorMessage(e));
        }
        if (Objects.isNull(responseEntity)) {
            return DEFAULT_DOWNLOAD_THREAD_LIMIT;
        }
        return Optional.ofNullable(responseEntity.getBody())
                .map(ClusterDetailInfo::getSourceClusters)
                .map(SourceClustersParams::getNodeCount)
                .map(nodeCount -> nodeCount * PER_CONTROLLER_THREAD_LIMIT)
                .orElse(DEFAULT_DOWNLOAD_THREAD_LIMIT);
    }
}
