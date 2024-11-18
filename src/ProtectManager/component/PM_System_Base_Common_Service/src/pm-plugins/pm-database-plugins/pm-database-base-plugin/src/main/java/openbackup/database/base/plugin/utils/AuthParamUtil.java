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
package openbackup.database.base.plugin.utils;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import com.huawei.oceanprotect.system.base.kerberos.entity.KerberosEntity;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.kerberos.model.KerberosBo;
import openbackup.system.base.util.Base64Util;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.MapUtils;

import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * 鉴权参数工具类
 *
 */
@Slf4j
public class AuthParamUtil {
    private AuthParamUtil() {
    }

    /**
     * 将ClickHouse备份或恢复拦截器任务中的认证信息转化返给框架
     *
     * @param auth auth
     * @param kerberosService kerberosService
     * @param encryptorService encryptorService
     * @param kerberosId kerberosId
     */
    public static void convertKerberosAuth(Authentication auth, KerberosService kerberosService, String kerberosId,
        EncryptorService encryptorService) {
        log.info("convert kerberos auth, kerberosId: {}", kerberosId);
        switch (auth.getAuthType()) {
            // 无认证（0）无需转换其他参数,认证类型设为NO_AUTH(1)
            case Authentication.NO_AUTH:
            case Authentication.APP_PASSWORD:
                break;

            // kerberos认证（5）需要转换keytab文件(或password)和krb5.conf文件装到环境信息中下发agent
            case Authentication.KERBEROS:
                addKerberosInfo(auth, getKerberosBo(kerberosService, kerberosId), encryptorService);
                break;

            // 其他认证类型抛错（ClickHouse有无认证、数据库认证和Kerberos认证3种）
            default:
                log.error("when execute ClickHouse task, auth type is not support, AuthType: {}", auth.getAuthType());
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Auth type is not support.");
        }
    }

    /**
     * 添加kerberos的认证参数
     *
     * @param auth kerberos认证信息
     * @param kerberosBo kerberosBo
     * @param encryptorService encryptorService
     */
    private static void addKerberosInfo(Authentication auth, KerberosBo kerberosBo, EncryptorService encryptorService) {
        Map<String, String> authExtendInfo = auth.getExtendInfo();

        // 将kerberos认证krb5.conf文件放入扩展参数中
        String krb5confFile = kerberosBo.getKrb5Content();
        authExtendInfo.put(DatabaseConstants.KERBEROS_KRB5_CONF, krb5confFile);

        // 将kerberos认证principal名称放入扩展参数中
        authExtendInfo.put(DatabaseConstants.EXTEND_INFO_KEY_PRINCIPAL, kerberosBo.getPrincipalName());
        auth.setAuthKey(kerberosBo.getPrincipalName());

        // 如果为keytab认证,将kerberos认证keytab文件放入扩展参数中,认证类型设为KERBEROS
        auth.setAuthType(Authentication.KERBEROS);
        if (DatabaseConstants.KERBEROS_MODEL_KEYTAB.equals(kerberosBo.getCreateModel())) {
            String keytabFile = kerberosBo.getKeytabContent();
            authExtendInfo.put(DatabaseConstants.KERBEROS_KEYTAB_FILE, keytabFile);
            authExtendInfo.put(DatabaseConstants.KERBEROS_CONFIG_MODEL, DatabaseConstants.KERBEROS_MODEL_KEYTAB);
        } else {
            // 如果为密码认证,将kerberos认证密码放入扩展参数中,agent自行解密,认证类型设为APP_PASSWORD
            auth.setAuthPwd(encryptorService.decrypt(kerberosBo.getPassword()));
            String secret = Base64Util.encryptToBase64(encryptorService.decrypt(kerberosBo.getPassword()));
            authExtendInfo.put(DatabaseConstants.KERBEROS_SECRET_KEY, secret);
            authExtendInfo.put(DatabaseConstants.KERBEROS_CONFIG_MODEL, DatabaseConstants.PASSWORD_MODEL);
        }
        auth.setExtendInfo(authExtendInfo);
    }

    /**
     * 获取KerberosBo
     *
     * @param kerberosService KerberosService
     * @param kerberosId kerberosId
     * @return KerberosBo
     */
    private static KerberosBo getKerberosBo(KerberosService kerberosService, String kerberosId) {
        KerberosEntity kerberosEntity = kerberosService.getKerberosById(kerberosId);
        if (Objects.isNull(kerberosEntity)) {
            log.error("KerberosEntity is null");
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST, "KerberosEntity is null.");
        }
        return kerberosEntity.castAsKerberosBo();
    }

    /**
     * 敏感信息最后一次使用结束后未在内存中移除
     *
     * @param children 集群下的所有节点
     */
    public static void removeSensitiveInfo(List<ProtectedResource> children) {
        children.stream().forEach(child -> {
            removeSensitiveInfo(child);
        });
    }

    /**
     * 敏感信息最后一次使用结束后未在内存中移除
     *
     * @param child 集群节点
     */
    public static void removeSensitiveInfo(ProtectedResource child) {
        Authentication auth = child.getAuth();
        if (!Objects.isNull(auth)) {
            auth.setAuthKey(null);
            auth.setAuthPwd(null);
            Map<String, String> authExtendInfo = auth.getExtendInfo();
            if (MapUtils.isNotEmpty(authExtendInfo)) {
                authExtendInfo.remove(DatabaseConstants.KERBEROS_KRB5_CONF);
                authExtendInfo.remove(DatabaseConstants.EXTEND_INFO_KEY_PRINCIPAL);
                authExtendInfo.remove(DatabaseConstants.KERBEROS_KEYTAB_FILE);
                authExtendInfo.remove(DatabaseConstants.KERBEROS_SECRET_KEY);
                authExtendInfo.remove(DatabaseConstants.KERBEROS_CONFIG_MODEL);
            }
        }
    }

    /**
     * 将的认证信息转化返给框架
     *
     * @param nodeResources nodeResources
     * @param kerberosService kerberosService
     * @param encryptorService encryptorService
     */
    public static void convertKerberosAuth(List<ProtectedResource> nodeResources, KerberosService kerberosService,
        EncryptorService encryptorService) {
        nodeResources.stream()
            .forEach(child -> convertKerberosAuth(child.getAuth(), kerberosService,
                child.getAuth().getExtendInfo().get(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID), encryptorService));
    }
}