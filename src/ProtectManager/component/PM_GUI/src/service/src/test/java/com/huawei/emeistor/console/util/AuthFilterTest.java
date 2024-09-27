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
package com.huawei.emeistor.console.util;

import com.huawei.emeistor.console.filter.AuthFilter;
import com.huawei.emeistor.console.util.RequestUtil;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;

import java.util.Arrays;
import java.util.regex.Pattern;

/**
 * 功能描述
 *
 * @author jwx701567
 * @since 2021-0-16
 */
@RunWith(PowerMockRunner.class)
@SpringBootTest(classes = {AuthFilter.class})
public class AuthFilterTest {
    @MockBean
    private RequestUtil requestUtil;

    private String authWhiteListStaticRegex =
            "/[0-9]+.[0-9a-zA-Z]+.js,/styles.[0-9a-zA-Z]+.css," +
                    "/runtime.[0-9a-zA-Z]+.js," +
                    "/polyfills.[0-9a-zA-Z]+.js," +
                    "/main.[0-9a-zA-Z]+.js,/HuaweiSans-Regular.[0-9a-zA-Z]+.ttf," +
                    "/assets/i18n/en-us/error-code/dorado_v6.json,";

    @Test
    public void check_white_list_static_regex_success() {
        String uri = "/assets/i18n/en-us/error-code/dorado_v6.json";
        String[] split = authWhiteListStaticRegex.split(",");
        boolean isTrue = Arrays.stream(split).anyMatch(regex -> Pattern.compile(regex).matcher(uri).matches());
        Assert.assertTrue(isTrue);
    }

}
