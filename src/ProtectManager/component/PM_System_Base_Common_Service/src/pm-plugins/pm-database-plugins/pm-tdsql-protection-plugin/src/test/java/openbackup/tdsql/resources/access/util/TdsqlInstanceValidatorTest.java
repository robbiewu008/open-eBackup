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
package openbackup.tdsql.resources.access.util;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.utils.json.JsonUtil;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 */
public class TdsqlInstanceValidatorTest {
    /**
     * 用例场景：创建/更新实例，进行参数检验
     * 前置条件：服务正常、实例节点正常，参数信息正确
     * 检查点：不报错
     */
    @Test
    public void test_check_tdsql_instance_success() {
        ProtectedResource protectedResource = getResource();
        TdsqlInstanceValidator.checkTdsqlInstance(protectedResource);
        Assert.assertEquals(protectedResource.getUuid(), "7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7");
    }

    private ProtectedResource getResource() {
        String json ="{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"test1\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"parentName\":\"\",\"parentUuid\":\"\",\"extendInfo\":{\"clusterInstanceInfo\":\"{\\\"groups\\\": [{\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"dataNodes\\\": [{\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"ip\\\": \\\"8.40.147.38\\\", \\\"port\\\": \\\"4002\\\", \\\"isMaster\\\": 1, \\\"defaultsFile\\\": \\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\", \\\"socket\\\": \\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"}, {\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"ip\\\": \\\"8.40.147.39\\\", \\\"port\\\": \\\"4002\\\", \\\"isMaster\\\": 0, \\\"defaultsFile\\\": \\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\", \\\"socket\\\": \\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"}, {\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"ip\\\": \\\"8.40.147.40\\\", \\\"port\\\": \\\"4002\\\", \\\"isMaster\\\": 0, \\\"defaultsFile\\\": \\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\", \\\"socket\\\": \\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"}]}], \\\"id\\\": \\\"set_1685328362_6\\\", \\\"type\\\": \\\"0\\\"}\"}}";
        return JsonUtil.read(json, ProtectedResource.class);
    }
}
