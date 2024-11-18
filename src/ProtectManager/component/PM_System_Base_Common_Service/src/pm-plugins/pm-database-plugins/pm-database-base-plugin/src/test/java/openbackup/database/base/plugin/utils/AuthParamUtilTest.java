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

import org.apache.commons.collections.MapUtils;
import org.junit.Assert;
import org.junit.Test;

import java.util.HashMap;
import java.util.Map;

/**
 * 鉴权参数工具测试类
 *
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