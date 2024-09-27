/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.database.base.plugin.utils;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;

import org.apache.commons.collections.MapUtils;
import org.junit.Assert;
import org.junit.Test;

import java.util.HashMap;
import java.util.Map;

/**
 * 鉴权参数工具测试类
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-02
 */
public class AuthParamUtilTest {
    /**
     * 用例名称：移除敏感信息成功
     * 前置条件：敏感信息存在
     * 检查点：相应值为null
     */
    @Test
    public void remove_sensitive_info_success() {
        ProtectedResource child = new ProtectedResource();
        Authentication auth = new Authentication();
        auth.setAuthKey("1");
        auth.setAuthPwd("2");
        Map<String, String> authExtendInfo = new HashMap<>();
        authExtendInfo.put(DatabaseConstants.KERBEROS_KRB5_CONF, "3");
        authExtendInfo.put(DatabaseConstants.EXTEND_INFO_KEY_PRINCIPAL, "4");
        authExtendInfo.put(DatabaseConstants.KERBEROS_KEYTAB_FILE, "5");
        authExtendInfo.put(DatabaseConstants.KERBEROS_SECRET_KEY, "6");
        authExtendInfo.put(DatabaseConstants.KERBEROS_CONFIG_MODEL, "7");
        auth.setExtendInfo(authExtendInfo);
        child.setAuth(auth);
        AuthParamUtil.removeSensitiveInfo(child);
        Assert.assertNull(auth.getAuthKey());
        Assert.assertNull(auth.getAuthPwd());
        Assert.assertNull(MapUtils.getString(authExtendInfo, DatabaseConstants.KERBEROS_KRB5_CONF));
        Assert.assertNull(MapUtils.getString(authExtendInfo, DatabaseConstants.EXTEND_INFO_KEY_PRINCIPAL));
        Assert.assertNull(MapUtils.getString(authExtendInfo, DatabaseConstants.KERBEROS_KEYTAB_FILE));
        Assert.assertNull(MapUtils.getString(authExtendInfo, DatabaseConstants.KERBEROS_SECRET_KEY));
        Assert.assertNull(MapUtils.getString(authExtendInfo, DatabaseConstants.KERBEROS_CONFIG_MODEL));
    }
}