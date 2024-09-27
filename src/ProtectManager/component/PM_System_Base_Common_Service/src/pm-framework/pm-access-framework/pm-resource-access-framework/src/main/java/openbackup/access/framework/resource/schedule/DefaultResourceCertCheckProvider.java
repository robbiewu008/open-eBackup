/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.schedule;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.utils.VerifyUtil;

import org.springframework.stereotype.Component;

import java.util.Map;
import java.util.Optional;

/**
 * 默认资源证书check provider
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/10/12
 */
@Component
public class DefaultResourceCertCheckProvider implements ResourceCertCheckProvider {
    /**
     * 从auth的扩展信息里获取证书
     *
     * @param protectedResource 资源
     * @return 证书内容
     */
    @Override
    public Optional<String> getCertContent(ProtectedResource protectedResource) {
        final Authentication auth = protectedResource.getAuth();
        if (VerifyUtil.isEmpty(auth)) {
            return Optional.empty();
        }
        final Map<String, String> extendInfo = auth.getExtendInfo();
        if (VerifyUtil.isEmpty(extendInfo)) {
            return Optional.empty();
        }
        return Optional.ofNullable(extendInfo.get(Constants.CERT_KEY));
    }

    /**
     * 从auth的扩展信息里获取吊销列表
     *
     * @param protectedResource 资源
     * @return 证吊销列表内容
     */
    @Override
    public Optional<String> getCrlContent(ProtectedResource protectedResource) {
        final Authentication auth = protectedResource.getAuth();
        if (VerifyUtil.isEmpty(auth)) {
            return Optional.empty();
        }
        final Map<String, String> extendInfo = auth.getExtendInfo();
        if (VerifyUtil.isEmpty(extendInfo)) {
            return Optional.empty();
        }
        return Optional.ofNullable(extendInfo.get(Constants.CRL_KEY));
    }

    /**
     * 适用范围
     *
     * @param protectedResource 资源
     * @return 是否适用
     */
    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return false;
    }
}
