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
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.util.ExceptionUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.io.FileSystemResource;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.util.CollectionUtils;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.util.StringUtils;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RequestCallback;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.text.MessageFormat;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * System Backup Controller for download and upload
 *
 */
@Slf4j
@RestController
@RequestMapping(ConfigConstant.CONSOLE + "/v1/sysbackup")
public class SystemBackupController extends AdvBaseController {
    private static final String SYSBACKUP_IMAGES = "/v1/sysbackup/images";

    private static final String DOWNLOAD_IMAGES = "/v1/sysbackup/images/{0}/action/download";

    private static final String SEARCH_DEFAULT_POLICY = "/v1/sysbackup/policy";

    /**
     * 2023.6.19 安全问题修改 新增子目录/uploadtmp 属主 pm-gui:nobody
     */
    private static final String UPLOAD_TEMP_PATH = "/opt/ProtectManager/";

    private static final String ZIP_FILE_SUFFIX = ".zip";

    private static final String ILLEGAL_FILENAME_CHAR = ":*?\"<>|";

    private static final String ILLEGAL_FILENAME_ONE = "./";

    private static final String ILLEGAL_FILENAME_TWO = ".\\";

    private static final String SYSBACKUP = "sysbackup_";

    private static final int TRY_TIMES = 3;

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private RequestUtil requestUtil;

    @Value("${api.gateway.endpoint}")
    private String sysbackupApi;

    /**
     * 上传镜像文件
     *
     * @param request 请求
     * @param response 响应
     * @param file MultipartFile
     * @param isNeedSignVerify 是否验签
     * @param password 加密密码
     * @throws IOException IO异常
     */
    @ExterAttack
    @PostMapping("/images")
    public void uploadBackup(HttpServletRequest request, HttpServletResponse response,
        @RequestParam("file") MultipartFile file, @RequestParam(value = "password", required = false) String password,
        @RequestParam(value = "needSignVerify", defaultValue = "true") boolean isNeedSignVerify) throws IOException {
        log.info("uploadBackup start");
        // 请求转发
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        HttpEntity<MultiValueMap<String, Object>> httpEntity1 = new HttpEntity<>(null, headers);
        restTemplate.exchange(sysbackupApi + SEARCH_DEFAULT_POLICY, HttpMethod.GET, httpEntity1, Map.class);

        // 1、创建本地中转文件
        verifyFileName(Objects.requireNonNull(file.getOriginalFilename()));
        String folderPath = UPLOAD_TEMP_PATH + SYSBACKUP + System.currentTimeMillis() + "/";
        File localFile = this.createLocalTempFile(file, folderPath);
        try {
            log.info("local file created, uploadBackup start");
            // 2、转发base_common服务处理
            HttpEntity<MultiValueMap<String, FileSystemResource>> httpEntity = this.buildUploadHttpEntity(request,
                localFile);
            ResponseEntity<Object> responseEntity;
            if (org.apache.commons.lang3.StringUtils.isBlank(password)) {
                responseEntity = restTemplate.postForEntity(super.normalizeForString(sysbackupApi + SYSBACKUP_IMAGES),
                    httpEntity, Object.class);
            } else {
                responseEntity = restTemplate.postForEntity(super.normalizeForString(
                        sysbackupApi + SYSBACKUP_IMAGES + "?password=" + password
                            + "&needSignVerify=" + isNeedSignVerify),
                    httpEntity, Object.class);
            }
            response.setStatus(responseEntity.getStatusCodeValue());
            log.info("uploadBackup end");
        } finally {
            // 无论是否处理成功，需要将本地的临时文件删除
            deleteUploadTempFile(folderPath);
        }
    }

    /**
     * 删除上传过程临时目录
     *
     * @param path 临时目录
     * @throws IOException exception
     */
    private void deleteUploadTempFile(String path) throws IOException {
        // 删除目录下文件
        File folder = new File(path);
        int times = 0;
        // 尝试三次
        while (times < TRY_TIMES) {
            try {
                FileUtils.deleteDirectory(folder);
                break;
            } catch (IOException e) {
                log.error("error to delete the file: {}", folder.getCanonicalPath(), ExceptionUtil.getErrorMessage(e));
                times++;
            }
        }
    }

    private HttpEntity<MultiValueMap<String, FileSystemResource>> buildUploadHttpEntity(HttpServletRequest request,
        File localFile) {
        FileSystemResource fileResource = new FileSystemResource(localFile);
        MultiValueMap<String, FileSystemResource> map = new LinkedMultiValueMap<>();

        // 这里需要注意，key需要和处理接口要求的参数一致
        map.add("file", fileResource);

        return super.getHttpEntity(map, request);
    }

    private File createLocalTempFile(MultipartFile file, String folderPath) throws IOException {
        File folder = new File(folderPath);
        if (!folder.exists()) {
            folder.mkdirs();
        }
        File localTempFile = new File(folderPath + System.currentTimeMillis() + ZIP_FILE_SUFFIX);
        log.info("createLocalTempFile localTempFile :{}", localTempFile.getAbsoluteFile());
        try (FileOutputStream fileOutputStream = FileUtils.openOutputStream(localTempFile);
            InputStream inputStream = file.getInputStream()) {
            IOUtils.copy(inputStream, fileOutputStream);
        }
        return localTempFile;
    }

    /**
     * 检查文件名称是否合法(防止解压绕过等）
     *
     * @param fileName 文件名称
     */
    private void verifyFileName(String fileName) {
        List<Integer> collect = ILLEGAL_FILENAME_CHAR.chars().boxed().collect(Collectors.toList());
        fileName.chars().boxed().forEach(intChar -> {
            if (collect.contains(intChar)) {
                log.error("file name illegal. fileName:{}", fileName);
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "file name illegal.");
            }
        });
        // 非法文件名
        if (fileName.contains(ILLEGAL_FILENAME_ONE) || fileName.contains(ILLEGAL_FILENAME_TWO)) {
            log.error("file name illegal. fileName:{}", fileName);
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "file name illegal.");
        }
    }

    /**
     * 下载镜像文件
     *
     * @param imagesId 备份id
     * @param response 响应
     */
    @GetMapping("/images/{imagesId}/action/download")
    public void downloadBackup(HttpServletResponse response, @PathVariable("imagesId") long imagesId) {
        // 本地中转文件目标
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        HttpEntity<MultiValueMap<String, Object>> httpEntity1 = new HttpEntity<>(null, headers);
        // 转发url
        String detailUrl = sysbackupApi + SYSBACKUP_IMAGES + "/" + imagesId;
        ResponseEntity<Map> entity = restTemplate.exchange(super.normalizeForString(detailUrl), HttpMethod.GET,
            httpEntity1, Map.class);
        if (CollectionUtils.isEmpty(entity.getBody())) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "Error search backup detail info.");
        }
        String fileName = entity.getBody().getOrDefault("backupPath", imagesId + ZIP_FILE_SUFFIX).toString();
        if (!StringUtils.isEmpty(fileName)) {
            fileName = Optional.of(fileName)
                .map(name -> new String(name.getBytes(StandardCharsets.UTF_8), StandardCharsets.ISO_8859_1))
                .orElseGet(String::new);
        }
        response.addHeader("Content-Disposition", "attachment;filename=" + fileName);
        response.setContentType("application/x-download");
        response.setCharacterEncoding("UTF-8");
        response.setHeader("Pragma", "no-cache");
        response.setHeader("Cache-Control", "no-store, must-revalidate");
        // 对响应进行流式处理而不是将其全部加载到内存中、
        String downloadUrl = MessageFormat.format(sysbackupApi + DOWNLOAD_IMAGES, imagesId);
        restTemplate.execute(super.normalizeForString(downloadUrl), HttpMethod.GET, generateRequestCallback(),
            clientHttpResponse -> {
                try (InputStream fileInputStream = clientHttpResponse.getBody();
                    ServletOutputStream outputStream = response.getOutputStream()) {
                    IOUtils.copy(fileInputStream, outputStream);
                } catch (IOException e) {
                    throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
                        "Error to download file from local.", e);
                }
                return null;
            });
    }

    private RequestCallback generateRequestCallback() {
        Map<String, String> headValueMap = requestUtil.getForwardHeaderAndValidCsrf().toSingleValueMap();
        return request -> {
            request.getHeaders().setAll(headValueMap);
            request.getHeaders().setAccept(Arrays.asList(MediaType.APPLICATION_OCTET_STREAM, MediaType.ALL));
        };
    }
}
