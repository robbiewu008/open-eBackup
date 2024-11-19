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
package openbackup.access.framework.resource.schedule;

import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.ResourceCertConstant;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.access.framework.resource.util.ResourceCertAlarmUtil;
import openbackup.access.framework.resource.util.ResourceCertUtil;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.pack.lock.Lock;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.AdapterUtils;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.TimeUnit;

/**
 * 资源证书的定时调度
 *
 */
@Slf4j
@Component
public class ResourceCertSchedule {
    private static final String ENV_CERT_CHECK_LOCK_KEY = "/env_cert_validity_check";

    private final ProviderManager providerManager;

    private final ResourceService resourceService;

    private final CommonAlarmService commonAlarmService;

    private final EncryptorService encryptorService;

    private final DefaultResourceCertCheckProvider defaultResourceCertCheckProvider;

    private LockService lockService;

    private DeployTypeService deployTypeService;

    /**
     * 构造函数
     *
     * @param resourceService resourceService
     * @param commonAlarmService resourceService
     * @param encryptorService encryptorService
     * @param providerManager providerManager
     * @param defaultResourceCertCheckProvider defaultResourceCertCheckProvider
     */
    public ResourceCertSchedule(ResourceService resourceService, CommonAlarmService commonAlarmService,
        EncryptorService encryptorService, ProviderManager providerManager,
        DefaultResourceCertCheckProvider defaultResourceCertCheckProvider) {
        this.resourceService = resourceService;
        this.commonAlarmService = commonAlarmService;
        this.encryptorService = encryptorService;
        this.providerManager = providerManager;
        this.defaultResourceCertCheckProvider = defaultResourceCertCheckProvider;
    }

    @Autowired
    public void setLockService(LockService lockService) {
        this.lockService = lockService;
    }

    @Autowired
    public void setDeployTypeService(DeployTypeService deployTypeService) {
        this.deployTypeService = deployTypeService;
    }

    /**
     * 定时检验（每天凌晨12点）
     */
    @Scheduled(cron = "0 0 0 * * ?")
    public void execute() {
        log.info("Start check env crt.");
        Lock lock = lockService.createSQLDistributeLock(ENV_CERT_CHECK_LOCK_KEY);
        if (!lock.tryLock(1, TimeUnit.SECONDS)) {
            log.info("another control is checking env certificate validity.");
            return;
        }
        try {
            ResourceQueryParams context = new ResourceQueryParams();
            int page = 0;
            int size = 200;
            context.setSize(size);
            Map<String, Object> conditions = new HashMap<>();
            conditions.put("discriminator", ProtectedResourcePo.ENVIRONMENTS_DISCRIMINATOR);
            context.setConditions(conditions);
            PageListResponse<ProtectedResource> response;
            do {
                context.setPage(page);
                response = resourceService.query(context);
                for (ProtectedResource protectedResource : response.getRecords()) {
                    checkResourceCert(protectedResource);
                }
                page++;
            } while (page * size < response.getTotalCount());
            log.info("End check env crt.");
        } finally {
            lock.unlock();
        }
    }

    private void checkResourceCert(ProtectedResource protectedResource) {
        String resourceName = protectedResource.getName();
        ResourceCertCheckProvider provider = providerManager.findProvider(ResourceCertCheckProvider.class,
                protectedResource, null);
        if (provider == null) {
            provider = defaultResourceCertCheckProvider;
        }
        // 检验证书是否有效
        provider.getCertContent(protectedResource).ifPresent(certificationEncrypt -> {
            log.info("Start check cert validity in env: {}", resourceName);
            String certification = encryptorService.decrypt(certificationEncrypt);
            try {
                if (!ResourceCertUtil.checkCertificateIsValid(certification, resourceName)) {
                    commonAlarmService.generateAlarm(
                        ResourceCertAlarmUtil.genResourceCertExpiredAlarm(ResourceCertConstant.CERT_EXPIRED_ID,
                            resourceName, deployTypeService.isCyberEngine() ? AdapterUtils.convertSubType(
                                protectedResource.getSubType()) : protectedResource.getSubType(),
                            protectedResource.getUuid()));
                    log.info("Env: {} send cert expire alarm success", resourceName);
                }
            } finally {
                StringUtil.clean(certification);
            }
        });
        // 检验证书吊销列表是否有效
        provider.getCrlContent(protectedResource).ifPresent(revocationListEncrypt -> {
            log.info("Start check crl validity in env: {}", resourceName);
            String revocationList = encryptorService.decrypt(revocationListEncrypt);
            try {
                if (!ResourceCertUtil.checkCrlIsValid(revocationList, resourceName)) {
                    commonAlarmService.generateAlarm(
                        ResourceCertAlarmUtil.genResourceCertExpiredAlarm(ResourceCertConstant.CRL_EXPIRED_ID,
                            resourceName, deployTypeService.isCyberEngine() ? AdapterUtils.convertSubType(
                                protectedResource.getSubType()) : protectedResource.getSubType(),
                            protectedResource.getUuid()));
                    log.info("Env: {} send crl expire alarm success", resourceName);
                }
            } finally {
                StringUtil.clean(revocationList);
            }
        });
    }
}
