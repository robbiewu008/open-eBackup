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
package openbackup.system.base.sdk.agent.config;

import openbackup.system.base.common.rest.CommonDecoder;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.config.FeignClientConfig;

import feign.Feign;
import feign.codec.Decoder;
import feign.codec.ErrorDecoder;
import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Bean;

/**
 * 功能描述
 *
 */
@Slf4j
public class AgentConfiguration {
    @Autowired
    private FeignClientConfig feignClientConfig;

    /**
     * Feign builder feign . builder.
     *
     * @return the feign . builder
     */
    @Bean("AgentConfigBuilder")
    public Feign.Builder feignBuilder() {
        return FeignBuilder.getDefaultRetryableBuilder().client(feignClientConfig.getInternalClient());
    }

    /**
     * errorDecoder
     *
     * @return errorDecoder
     */
    @Bean
    public ErrorDecoder errorDecoder() {
        return CommonDecoder::errorDecode;
    }

    /**
     * docoder
     *
     * @return docoder
     */
    @Bean
    public Decoder decoder() {
        return CommonDecoder.decoder();
    }
}
