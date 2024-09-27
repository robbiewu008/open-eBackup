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
package openbackup.data.access.framework.core.common.util;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.util.RequestUriUtil;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import java.net.URI;

/**
 * RequestUriUtil Test类
 *
 * @author z30009433
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-24
 */
public class RequestUriUtilTest {
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    /**
     * 用例场景：正常获取uri场景
     * 前置条件：endpoint不为空， port在（1-65535）中
     * 检查点：正常获取uri成功
     */
    @Test
    public void get_request_uri_success() {
        URI uri = RequestUriUtil.getRequestUri("protectengine", 8090);
        Assert.assertEquals(uri.toString(), "https://protectengine:8090");
    }

    /**
     * 用例场景：endpoint为空时，为错误输入，抛错
     * 前置条件：endpoint为空
     * 检查点：endpoint为空，抛错
     */
    @Test
    public void get_request_uri_if_endpoint_is_empty() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("ip can not be empty.");

        RequestUriUtil.getRequestUri("", 0);
    }

    /**
     * 用例场景：port超过范围（1-65535），为错误输入，抛错
     * 前置条件：port为
     * 检查点：port超过范围时，抛错
     */
    @Test
    public void get_request_uri_if_port_out_of_range() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("port can not be empty and out of range.");

        RequestUriUtil.getRequestUri("protectengine", 0);
    }
}