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
package openbackup.data.access.framework.protection.service.repository.strategies;

import com.huawei.oceanprotect.repository.s3.entity.S3Storage;
import com.huawei.oceanprotect.repository.s3.service.S3StorageService;
import com.huawei.oceanprotect.system.base.cert.service.ObjectCertDependencyService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.Proxy;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.StorageTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;

import org.apache.commons.lang3.StringUtils;
import org.apache.hc.core5.http.URIScheme;
import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * S3存储库的策略类
 *
 **/
@Component("s3RepositoryStrategy")
@Slf4j
public class S3RepositoryStrategy implements RepositoryStrategy {
    private static final String AZURE_STR_CONNECT_DOMAIN_REG = "EndpointSuffix=(https?://)?([^/:]+)";
    private static final String AZURE_STR_CONNECT_IP_REG = "BlobEndpoint=(https?://)?([^/:]+)(:\\d+)?";

    private final S3StorageService s3StorageService;

    private final ObjectCertDependencyService objectCertDependencyService;


    public S3RepositoryStrategy(S3StorageService s3StorageService,
        ObjectCertDependencyService objectCertDependencyService) {
        this.s3StorageService = s3StorageService;
        this.objectCertDependencyService = objectCertDependencyService;
    }

    @Override
    public Authentication getAuthentication(String repositoryId) {
        // S3存储用AK/SK填入用户名/密码字段进行认证
        final S3Storage s3Storage = queryStorage(repositoryId);
        return covertToAuthentication(s3Storage);
    }

    private Authentication covertToAuthentication(S3Storage s3Storage) {
        Authentication newAuthInfo = new Authentication();
        newAuthInfo.setAuthKey(s3Storage.getAk());
        newAuthInfo.setAuthPwd(s3Storage.getSk());
        newAuthInfo.setAuthType(Authentication.AKSK);
        if (s3Storage.isHttps() || Integer.valueOf(1).equals(s3Storage.getConnectType())) {
            Map<String, String> map = new HashMap<>();
            map.put("certName", objectCertDependencyService.getCertNameByObjectId(s3Storage.getId()).getCertName());
            newAuthInfo.setExtendInfo(map);
        }
        return newAuthInfo;
    }

    @Override
    public Endpoint getEndpoint(String repositoryId) {
        final S3Storage s3Storage = queryStorage(repositoryId);
        return new Endpoint(s3Storage.getId(), s3Storage.getEndpoint(), s3Storage.getPort());
    }

    @Override
    public StorageRepository getRepository(BaseStorageRepository baseRepository) {
        String repositoryId = baseRepository.getId();
        StorageRepository storageRepository = new StorageRepository();
        final S3Storage storageInfo = queryStorage(repositoryId);
        String sk = storageInfo.getSk();
        if (storageInfo.getCloudType() == StorageTypeEnum.AZURE_BLOB.getStorageType()
                && Integer.valueOf(1).equals(storageInfo.getConnectType())) {
            String domain = extractDomain(sk);
            String ip = extractIp(sk);
            String endPoint = VerifyUtil.isEmpty(domain) ? ip : domain;
            storageInfo.setEndpoint(endPoint);
        }

        final Endpoint endpoint = new Endpoint(storageInfo.getId(), storageInfo.getEndpoint(), storageInfo.getPort());
        // s3存储extendInfo为空，path为数据桶名称
        BeanUtils.copyProperties(baseRepository, storageRepository);
        storageRepository.setPath(storageInfo.getDataBucket());
        storageRepository.setEndpoint(endpoint);
        storageRepository.setProtocol(RepositoryProtocolEnum.S3.getProtocol());
        storageRepository.setLocal(Boolean.FALSE);
        storageRepository.setAuth(covertToAuthentication(storageInfo));
        storageRepository.setProxy(covertToProxy(storageInfo));
        storageRepository.setTransProtocol(storageInfo.isHttps() ? URIScheme.HTTPS.getId() : URIScheme.HTTP.getId());
        storageRepository.setCloudType(storageInfo.getCloudType());
        storageRepository.setConnectType(storageInfo.getConnectType());
        return storageRepository;
    }

    private String extractDomain(String connStr) {
        Pattern pattern = Pattern.compile(AZURE_STR_CONNECT_DOMAIN_REG);
        Matcher matcher = pattern.matcher(connStr);
        return matcher.find() ? matcher.group(2) : StringUtils.EMPTY;
    }

    private String extractIp(String connStr) {
        Pattern pattern = Pattern.compile(AZURE_STR_CONNECT_IP_REG);
        Matcher matcher = pattern.matcher(connStr);
        return matcher.find() ? matcher.group(2) : StringUtils.EMPTY;
    }

    private Proxy covertToProxy(S3Storage s3Storage) {
        Proxy proxy = new Proxy();
        proxy.setEnabled(s3Storage.isProxyEnable());
        if (s3Storage.isProxyEnable()) {
            proxy.setHostName(s3Storage.getProxyUrl());
            proxy.setPort(Integer.parseInt(s3Storage.getProxyPort()));
            proxy.setUserName(s3Storage.getProxyUserName());
            proxy.setPassword(s3Storage.getProxyUserPwd());
        }
        return proxy;
    }

    private S3Storage queryStorage(String storageId) {
        final S3Storage s3Storage = s3StorageService.queryS3Storage(storageId);
        if (s3Storage == null) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "S3 storage, storageId:" + storageId + "not exists");
        }
        return s3Storage;
    }
}
