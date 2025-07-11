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
package openbackup.access.framework.resource.persistence.model;

import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.utils.JSONObject;

import com.fasterxml.jackson.core.JsonProcessingException;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

import java.sql.Timestamp;
import java.util.Collections;

/**
 * Protected Resource Po Test
 *
 */
public class ProtectedResourcePoTest {
    /**
     * 用例名称：验证protectedObject扩展属性装换是否正确。<br/>
     * 前置条件：protectedObject扩展属性为双重json序列化参数。<br/>
     * check点：转化后的扩展参数与预期的map对象一致。
     *
     * @throws JsonProcessingException json processing exception
     */
    @Test
    public void test_to_protected_resource() throws JsonProcessingException {
        ProtectedObjectPo protectedObjectPo = new ProtectedObjectPo();
        protectedObjectPo.setSlaCompliance(true);
        protectedObjectPo.setExtParameters(
            JSONObject.RAW_OBJ_MAPPER.writeValueAsString(new JSONObject().set("key", "value").toString()));
        ProtectedResourcePo resourcePo = new ProtectedResourcePo();
        resourcePo.setProtectedObjectPo(protectedObjectPo);
        resourcePo.setCreatedTime(new Timestamp(0));
        resourcePo.setUserId("");
        ProtectedResource protectedResource = resourcePo.toProtectedResource();
        ProtectedObject po = protectedResource.getProtectedObject();
        Assertions.assertNotNull(po);
        Assertions.assertNotNull(po.getExtParameters());
        Assertions.assertTrue(po.getSlaCompliance());
        Assertions.assertEquals(Collections.singletonMap("key", "value"),
            protectedResource.getProtectedObject().getExtParameters());

        protectedObjectPo.setExtParameters(null);
        protectedResource = resourcePo.toProtectedResource();
        Assertions.assertNotNull(protectedResource.getProtectedObject());
        Assertions.assertTrue(protectedResource.getProtectedObject().getExtParameters().isEmpty());
        Assertions.assertNull(protectedResource.getUserId());
    }
}
