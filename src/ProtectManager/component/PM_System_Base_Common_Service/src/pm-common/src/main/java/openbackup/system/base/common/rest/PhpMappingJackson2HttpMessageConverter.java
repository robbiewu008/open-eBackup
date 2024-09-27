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
