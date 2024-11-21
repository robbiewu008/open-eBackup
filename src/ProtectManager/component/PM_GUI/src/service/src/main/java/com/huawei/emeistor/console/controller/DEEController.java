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

import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.controller.request.DownloadAbnormalRequest;
import com.huawei.emeistor.console.controller.request.ExportSuspectFileReportRequest;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.util.ExceptionUtil;
import com.huawei.emeistor.console.util.NormalizerUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import com.alibaba.fastjson.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.io.IOUtils;
import org.apache.http.HttpStatus;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;

import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletResponse;
import javax.validation.Valid;

/**
 * 转发DEE接口
 *
 */
@RestController
@RequestMapping(ConfigConstant.CONSOLE)
@Slf4j
public class DEEController {
    private static final String ANTI_DOWNLOAD_FILE_URL = "/v1/anti/ransomware/model";

    private static final String IO_DETECT_REPORT_EXPORT_URL = "/v1/anti/ransomware/io-detect/report/export";

    private static final String DETECT_ABNORMAL_FILE_URL = "/v1/anti/ransomware/detect/copy/action/abnormal/download";

    private static final String DOWNLOAD_RFI_FILE_URL = "/v1/flr/indexes/action/download-rfi";
    private static final String NODE_NAME = "NODE_NAME";

    private static final String CSV_SUFFIX = ".csv";

    private static final String ABNORMAL_FILE = "abnormal_file";

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private HttpServletResponse response;

    @Autowired
    private RequestUtil requestUtil;

    @Value("${api.gateway.endpoint}")
    private String antiUrl;

    /**
     * 上传检测模型
     *
     * @param modelFile model file
     * @throws IOException 服务异常
     */
    @ExterAttack
    @PostMapping("/v1/anti/ransomware/model")
    public void addModelInfo(@RequestParam("modelFile") MultipartFile modelFile) {
        log.info("push model to dee start");
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        headers.setContentType(MediaType.MULTIPART_FORM_DATA);
        String requestUrl = NormalizerUtil.normalizeForString(antiUrl + ANTI_DOWNLOAD_FILE_URL);

        // 转发DEE模型
        MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
        Resource uploadResource = modelFile.getResource();
        map.add("modelFile", uploadResource);
        HttpEntity<MultiValueMap<String, Object>> httpEntity = new HttpEntity<>(map, headers);
        ResponseEntity<Object> responseEntity = restTemplate.postForEntity(requestUrl, httpEntity, Object.class);

        // GUI转发后端后，如果网关未配置(404/405)，则直接设置响应码返回
        if ((responseEntity.getStatusCodeValue() == HttpStatus.SC_NOT_FOUND)
            || (responseEntity.getStatusCodeValue() == HttpStatus.SC_METHOD_NOT_ALLOWED)) {
            response.setStatus(responseEntity.getStatusCodeValue());
            return;
        }

        // 返回数据处理
        response.setStatus(HttpStatus.SC_OK);
        String deeResponse = JSONObject.toJSONString(responseEntity.getBody());
        try (ServletOutputStream outputStream = response.getOutputStream()) {
            outputStream.write(deeResponse.getBytes(StandardCharsets.UTF_8));
            outputStream.flush();
        } catch (IOException ioException) {
            log.error("push model to dee error.dee response");
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR);
        }
        log.info("push model to dee end.");
    }

    /**
     * downloadIoDetectReport
     *
     * @param request request
     * @param response response
     */
    @ExterAttack
    @GetMapping(IO_DETECT_REPORT_EXPORT_URL)
    public void downloadIoDetectReport(
            @Valid ExportSuspectFileReportRequest request, HttpServletResponse response) {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        String ioDetectReportDownloadUrl = NormalizerUtil.normalizeForString(
                antiUrl + IO_DETECT_REPORT_EXPORT_URL + "?deviceId=" + request.getDeviceId() + "&vstoreId="
                        + request.getVstoreId() + "&fileSystemName=" + request.getFileSystemName() + "&snapShotName="
                        + request.getSnapShotName() + "&lang=" + request.getLang());
        String fileName = System.getenv(NODE_NAME) + CSV_SUFFIX;
        restTemplate.execute(ioDetectReportDownloadUrl, HttpMethod.GET,
                clientHttpRequest -> clientHttpRequest.getHeaders().setAll(headers.toSingleValueMap()),
                clientHttpResponse -> {
                    try (InputStream inputStream = clientHttpResponse.getBody();
                         OutputStream outputStream = response.getOutputStream()) {
                        response.setStatus(clientHttpResponse.getStatusCode().value());
                        response.setContentType("application/x-download");
                        response.setCharacterEncoding("UTF-8");
                        response.setHeader("Pragma", "no-cache");
                        response.setHeader("Cache-Control", "no-store, must-revalidate");
                        response.addHeader("Content-Disposition",
                                "attachment;filename=" + request.getSnapShotName() + "_"
                                        + request.getFileSystemName() + "_" + fileName);
                        IOUtils.copy(inputStream, outputStream);
                    } catch (IOException e) {
                        log.error("Fail to export suspect files.", ExceptionUtil.getErrorMessage(e));
                        throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, e);
                    }
                    log.info("Export suspect files.");
                    return null;
                }
        );
    }

    /**
     * downloadIoDetectReport
     *
     * @param request request
     * @param response response
     */
    @ExterAttack
    @GetMapping(DETECT_ABNORMAL_FILE_URL)
    public void downloadAbnormalReport(
        @Valid DownloadAbnormalRequest request, HttpServletResponse response) {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        String copyId = request.getCopyId().split("/")[0];
        String abnormalDownloadUrl = NormalizerUtil.normalizeForString(
            antiUrl + DETECT_ABNORMAL_FILE_URL + "?copyId=" + copyId);
        String fileName = ABNORMAL_FILE + CSV_SUFFIX;
        restTemplate.execute(abnormalDownloadUrl, HttpMethod.GET,
            clientHttpRequest -> clientHttpRequest.getHeaders().setAll(headers.toSingleValueMap()),
            clientHttpResponse -> {
                try (InputStream inputStream = clientHttpResponse.getBody();
                    OutputStream outputStream = response.getOutputStream()) {
                    response.setStatus(clientHttpResponse.getStatusCode().value());
                    response.setContentType("application/x-download");
                    response.setCharacterEncoding("UTF-8");
                    response.setHeader("Pragma", "no-cache");
                    response.setHeader("Cache-Control", "no-store, must-revalidate");
                    response.addHeader("Content-Disposition",
                        "attachment;filename=" + request.getCopyId() + "_" + fileName);
                    IOUtils.copy(inputStream, outputStream);
                } catch (IOException e) {
                    log.error("Fail to export suspect files.", ExceptionUtil.getErrorMessage(e));
                    throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, e);
                }
                log.info("Export suspect files.");
                return null;
            }
        );
    }

    /**
     * rfi文件下载
     *
     * @param copyId copyId
     * @param response response
     */
    @ExterAttack
    @GetMapping(DOWNLOAD_RFI_FILE_URL)
    public void downloadRfiFile(
            @RequestParam(name = "uuid", required = true) String copyId, HttpServletResponse response) {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        String rfiDownloadUrl = NormalizerUtil.normalizeForString(
                antiUrl + DOWNLOAD_RFI_FILE_URL + "?uuid=" + copyId);
        String fileName = "RFI_" + copyId + ".json";
        restTemplate.execute(rfiDownloadUrl, HttpMethod.GET,
                clientHttpRequest -> clientHttpRequest.getHeaders().setAll(headers.toSingleValueMap()),
                clientHttpResponse -> {
                    try (InputStream inputStream = clientHttpResponse.getBody();
                         OutputStream outputStream = response.getOutputStream()) {
                        response.setStatus(clientHttpResponse.getStatusCode().value());
                        response.setContentType("application/x-download");
                        response.setCharacterEncoding("UTF-8");
                        response.setHeader("Pragma", "no-cache");
                        response.setHeader("Cache-Control", "no-store, must-revalidate");
                        response.addHeader("Content-Disposition",
                                "attachment;filename=" + fileName);
                        IOUtils.copy(inputStream, outputStream);
                    } catch (IOException e) {
                        log.error("Fail to export suspect files.", ExceptionUtil.getErrorMessage(e));
                        throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, e);
                    }
                    log.info("Export suspect files.");
                    return null;
                }
        );
    }
}