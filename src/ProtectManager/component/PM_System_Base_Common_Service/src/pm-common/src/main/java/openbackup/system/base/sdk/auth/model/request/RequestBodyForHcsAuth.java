/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model.request;

import lombok.Getter;
import lombok.Setter;

/**
 * 用于获取token的请求体
 *
 * @author l30044826
 * @since 2023-07-07
 */
@Getter
@Setter
public class RequestBodyForHcsAuth {
    private HcsTokenAuth auth;
}
