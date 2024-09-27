/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
