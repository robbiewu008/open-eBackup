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
package openbackup.system.base.common.rest;

import openbackup.system.base.common.rest.PhpMappingJackson2HttpMessageConverter;

import org.junit.Assert;
import org.junit.Test;
import org.springframework.http.MediaType;

import java.util.Arrays;
import java.util.List;

/**
 * Php Mapping Jackson Http Message Converter Test
 *
 * @author l00272247
 * @since 2021-02-23
 */
public class PhpMappingJackson2HttpMessageConverterTest {
    @Test
    public void test_media_types() {
        List<MediaType> expect = Arrays.asList(MediaType.APPLICATION_OCTET_STREAM, MediaType.TEXT_PLAIN,
            MediaType.TEXT_PLAIN, MediaType.APPLICATION_JSON);
        List<MediaType> actual = PhpMappingJackson2HttpMessageConverter.getDefaultMediaTypes();
        boolean matched = actual.stream().anyMatch(type -> expect.stream().noneMatch(type::isCompatibleWith));
        Assert.assertTrue(matched);
    }
}
