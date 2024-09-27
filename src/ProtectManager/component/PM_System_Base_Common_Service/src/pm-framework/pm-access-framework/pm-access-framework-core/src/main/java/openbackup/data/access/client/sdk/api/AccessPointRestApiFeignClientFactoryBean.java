/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api;

import openbackup.system.base.common.rest.FeignClientFactoryBean;

import org.springframework.stereotype.Component;

/**
 * Access Point Rest Api Feign Client Factory Bean
 *
 * @author l00272247
 * @version [OceanProtect A8000 1.1.0]
 * @since 2020-12-18
 */
@Component
public class AccessPointRestApiFeignClientFactoryBean extends FeignClientFactoryBean<AccessPointRestApi> {
}
