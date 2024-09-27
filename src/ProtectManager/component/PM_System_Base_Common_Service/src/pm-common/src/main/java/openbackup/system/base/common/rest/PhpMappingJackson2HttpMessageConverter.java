/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.rest;

import org.springframework.http.MediaType;
import org.springframework.http.converter.json.MappingJackson2HttpMessageConverter;

import java.util.ArrayList;
import java.util.List;

/**
 * Php Mapping Jackson2 Http Message Converter
 *
 * @author l00272247
 * @since 2021-02-23
 */
public class PhpMappingJackson2HttpMessageConverter extends MappingJackson2HttpMessageConverter {
    /**
     * constructor
     */
    public PhpMappingJackson2HttpMessageConverter() {
        List<MediaType> mediaTypes = getDefaultMediaTypes();
        setSupportedMediaTypes(mediaTypes);
    }

    /**
     * default media types
     *
     * @return default media types
     */
    public static List<MediaType> getDefaultMediaTypes() {
        List<MediaType> mediaTypes = new ArrayList<>();
        mediaTypes.add(MediaType.valueOf(MediaType.APPLICATION_OCTET_STREAM_VALUE));
        mediaTypes.add(MediaType.valueOf(MediaType.TEXT_HTML_VALUE + ";charset=UTF-8"));
        mediaTypes.add(MediaType.valueOf(MediaType.TEXT_PLAIN_VALUE + ";charset=UTF-8"));
        mediaTypes.add(MediaType.APPLICATION_JSON);
        return mediaTypes;
    }
}
