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

import openbackup.access.framework.resource.schedule.VmwareResourceCertCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.Constants;
import org.junit.Assert;
import org.junit.Test;

import java.util.HashMap;

/**
 * VmwareResourceCertCheckProvider Test
 *
 */
public class VmwareResourceCertCheckProviderTest {
    /**
     * 用例名称：获取vmware资源的证书和吊销列表。
     * 前置条件：资源是vmware。
     * check点：证书和吊销列表内容符合预期。
     */
    @Test
    public void get_cert_and_crl_content_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        final HashMap<String, String> map = new HashMap<>();
        map.put(Constants.CERT_KEY, "111");
        map.put(Constants.REVOCATION_LIST_VMWARE, "222");
        protectedResource.setExtendInfo(map);
        VmwareResourceCertCheckProvider vmwareResourceCertCheckProvider = new VmwareResourceCertCheckProvider();
        Assert.assertEquals("111", vmwareResourceCertCheckProvider.getCertContent(protectedResource).get());
        Assert.assertEquals("222", vmwareResourceCertCheckProvider.getCrlContent(protectedResource).get());
    }
}
