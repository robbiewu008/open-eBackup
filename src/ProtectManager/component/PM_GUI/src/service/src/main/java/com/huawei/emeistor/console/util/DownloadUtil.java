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

import com.huawei.emeistor.console.exterattack.ExterAttack;

import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.stereotype.Component;
import org.springframework.util.StreamUtils;
import org.springframework.web.client.RestTemplate;

import java.io.InputStream;
import java.io.OutputStream;

import javax.servlet.http.HttpServletResponse;

/**
 * 下载功能工具类
 *
 */
@Component
public class DownloadUtil {
    private static final Logger LOGGER = LoggerFactory.getLogger(DownloadUtil.class);

    private static final String TRANSFER_ENCODING = "Transfer-Encoding";

    private static final String DELIMITER = ",";

    @Autowired
    private HttpServletResponse response;

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private RequestUtil requestUtil;

    /**
     * 下载文件
     * <p>
     * if (!StringUtils.equals(TRANSFER_ENCODING, key)) 逻辑原因：
     * 解决大文件下载失败，traefik报错的问题。
     * 下载接口和DEE交互，跨POD和语言，DEE返回的http头会携带chunked，restTemplate在转出时又会追加一个chunked，
     * 导致trafik将请求断掉。
     * 详细解释见https://community.pivotal.io/s/article/Application-Chunked-Error?language=en_US
     * 报错：{"level":"debug","msg":"'500 Internal Server Error' caused by: net/http: HTTP/1.x transport
     * connection broken: too many transfer encodings: [\"chunked\"
     * \"chunked\"]","time":"2021-03-11T08:36:34Z"}
     *
     * @param requestUrl requestUrl
     */
    @ExterAttack
    public void download(String requestUrl) {
        LOGGER.info("requestUrl: {}", requestUrl);
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        restTemplate.execute(requestUrl, HttpMethod.GET, clientHttpRequest -> headers.toSingleValueMap()
            .forEach((key, value) -> clientHttpRequest.getHeaders().set(key, value)), clientHttpResponse -> {
            HttpHeaders responseHeaders = clientHttpResponse.getHeaders();
            responseHeaders.forEach((key, value) -> {
                if (!StringUtils.equals(TRANSFER_ENCODING, key)) {
                    response.setHeader(key, String.join(DELIMITER, value));
                }
            });
            try (OutputStream os = response.getOutputStream(); InputStream in = clientHttpResponse.getBody()) {
                StreamUtils.copy(in, os);
            }
            return null;
        });
    }
}
