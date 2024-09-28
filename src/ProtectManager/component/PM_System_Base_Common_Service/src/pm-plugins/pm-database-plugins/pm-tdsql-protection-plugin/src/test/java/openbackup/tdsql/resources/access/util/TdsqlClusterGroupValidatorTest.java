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
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;

import org.junit.Assert;
import org.junit.Test;

/**
 * TdsqlClusterGroupValidator测试用例
 *
 */
public class TdsqlClusterGroupValidatorTest {
    @Test
    public void test_check_tdsql_group_params_success() {
        ProtectedResource protectedResource = getRightResource();
        TdsqlClusterGroupValidator.checkTdsqlGroupParams(protectedResource);
        Assert.assertEquals(protectedResource.getUuid(), "7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7");
    }

    private ProtectedResource getRightResource() {
        String json  =  "{\n" + "  \"uuid\": \"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\n"
            + "  \"name\": \"group_1698889827_3\",\n" + "  \"parentUuid\": \"9e68a8f1-7ad4-3eef-a808-dce3b2062120\",\n"
            + "  \"type\": \"Database\",\n" + "  \"subType\": \"TDSQL-clusterGroup\",\n" + "  \"extendInfo\": {\n"
            + "    \"clusterGroupInfo\": \"{\\\"id\\\":\\\"group_1698889827_3\\\",\\\"name\\\":\\\"group_1698889827_3\\\",\\\"cluster\\\":\\\"9e68a8f1-7ad4-3eef-a808-dce3b2062120\\\",\\\"type\\\":\\\"1\\\",\\\"group\\\":{\\\"groupId\\\":\\\"group_1698889827_3\\\",\\\"setIds\\\":[\\\"set_1698890174_3\\\",\\\"set_1698890103_1\\\"],\\\"dataNodes\\\":[{\\\"ip\\\":\\\"8.40.168.190\\\",\\\"parentUuid\\\":\\\"dd15b622-fd7c-4a7c-9841-b0fb45b4201f\\\"},{\\\"ip\\\":\\\"8.40.168.191\\\",\\\"parentUuid\\\":\\\"ce56a464-3b7b-4016-88f2-58ae12fb6d1d\\\"},{\\\"ip\\\":\\\"8.40.168.192\\\",\\\"parentUuid\\\":\\\"c75146f7-7e2a-41d6-b110-28d0e22245ee\\\"}]}}\"\n"
            + "  }\n" + "}";
        return JsonUtil.read(json, ProtectedResource.class);
    }

    @Test(expected = LegoCheckedException.class)
    public void test_check_tdsql_group_set_params_failed() {
        ProtectedResource protectedResource = getSetErrorResource();
        TdsqlClusterGroupValidator.checkTdsqlGroupParams(protectedResource);
    }

    private ProtectedResource getSetErrorResource() {
        String json = "{\n" + "  \"uuid\": \"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\n"
            + "  \"name\": \"group_1698889827_3\",\n" + "  \"parentUuid\": \"9e68a8f1-7ad4-3eef-a808-dce3b2062120\",\n"
            + "  \"type\": \"Database\",\n" + "  \"subType\": \"TDSQL-clusterGroup\",\n" + "  \"extendInfo\": {\n"
            + "    \"clusterGroupInfo\": \"{\\\"id\\\":\\\"group_1698889827_3\\\",\\\"name\\\":\\\"group_1698889827_3\\\",\\\"cluster\\\":\\\"9e68a8f1-7ad4-3eef-a808-dce3b2062120\\\",\\\"type\\\":\\\"1\\\",\\\"group\\\":{\\\"groupId\\\":\\\"group_1698889827_3\\\",\\\"dataNodes\\\":[{\\\"ip\\\":\\\"8.40.168.190\\\",\\\"parentUuid\\\":\\\"dd15b622-fd7c-4a7c-9841-b0fb45b4201f\\\"},{\\\"ip\\\":\\\"8.40.168.191\\\",\\\"parentUuid\\\":\\\"ce56a464-3b7b-4016-88f2-58ae12fb6d1d\\\"}]}}\"\n"
            + "  }\n" + "}";
        return JsonUtil.read(json, ProtectedResource.class);
    }

    @Test(expected = LegoCheckedException.class)
    public void test_check_tdsql_group_datanode_params_failed() {
        ProtectedResource protectedResource = getDataNodeErrorResource();
        TdsqlClusterGroupValidator.checkTdsqlGroupParams(protectedResource);
    }

    private ProtectedResource getDataNodeErrorResource() {
        String json = "{\n" + "  \"uuid\": \"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\n"
            + "  \"name\": \"group_1698889827_3\",\n" + "  \"parentUuid\": \"9e68a8f1-7ad4-3eef-a808-dce3b2062120\",\n"
            + "  \"type\": \"Database\",\n" + "  \"subType\": \"TDSQL-clusterGroup\",\n" + "  \"extendInfo\": {\n"
            + "    \"clusterGroupInfo\": \"{\\\"id\\\":\\\"group_1698889827_3\\\",\\\"name\\\":\\\"group_1698889827_3\\\",\\\"cluster\\\":\\\"9e68a8f1-7ad4-3eef-a808-dce3b2062120\\\",\\\"type\\\":\\\"1\\\",\\\"group\\\":{\\\"groupId\\\":\\\"group_1698889827_3\\\",\\\"setIds\\\":[\\\"set_1698890174_3\\\",\\\"set_1698890103_1\\\"],\\\"dataNodes\\\":[{\\\"parentUuid\\\":\\\"dd15b622-fd7c-4a7c-9841-b0fb45b4201f\\\"},{\\\"ip\\\":\\\"8.40.168.191\\\",\\\"parentUuid\\\":\\\"ce56a464-3b7b-4016-88f2-58ae12fb6d1d\\\"}]}}\"\n"
            + "  }\n" + "}";
        return JsonUtil.read(json, ProtectedResource.class);
    }
}
