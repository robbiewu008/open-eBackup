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
package openbackup.goldendb.protection.access.util;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;

import openbackup.system.base.common.utils.json.JsonUtil;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 */
public class GoldenDbValidatorTest {

    @Test
    public void testCheckGoldenDbInstance() {
        GoldenDbInstanceValidator.checkGoldenDbInstance(getEnvironment());
        Assert.assertTrue(true);
    }

    private ProtectedEnvironment getEnvironment() {
        String json ="{\"name\":\"c5\",\"type\":\"Database\",\"subType\":\"GoldenDB-clusterInstance\",\"parentUuid\":\"1545cef9-9ba9-3838-a174-d32c3db842ca\",\"auth\":{\"authType\":2,\"authKey\":\"super\",\"authPwd\":\"Huawei@121\"},\"extendInfo\":{\"clusterInfo\":\"{\\\"id\\\":\\\"7\\\",\\\"name\\\":\\\"cluster5\\\",\\\"group\\\":[{\\\"groupId\\\":\\\"1\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"id\\\":\\\"12\\\",\\\"name\\\":\\\"DN11\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.106.157\\\",\\\"port\\\":\\\"5507\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup1\\\",\\\"parent\\\":null,\\\"parentUuid\\\":\\\"998c25a0-1254-458f-bbbe-5b7f74077dee\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.106.157)\\\",\\\"osUser\\\":\\\"zxdb15\\\"},{\\\"id\\\":\\\"13\\\",\\\"name\\\":\\\"DN12\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.106.157\\\",\\\"port\\\":\\\"5508\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup1\\\",\\\"parent\\\":null,\\\"parentUuid\\\":\\\"998c25a0-1254-458f-bbbe-5b7f74077dee\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.106.157)\\\",\\\"osUser\\\":\\\"zxdb16\\\"}]}],\\\"gtm\\\":[]}\",\"local_ini_cnf\":\"W2NvbW1vbl0KI1Jvb3QgZGlyIG9mIGJhY2t1cGluZzsKI3VuaXQ6IE5BLCByYW5nZTogTkEsIGRlZmF1bHQ6ICRIT01FL2JhY2t1cF9yb290CmJhY2t1cF9yb290ZGlyID0K\"},\"dependencies\":{\"agents\":[{\"uuid\":\"998c25a0-1254-458f-bbbe-5b7f74077dee\"}],\"-agents\":[]}}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }
}
