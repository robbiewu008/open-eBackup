/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * The CheckResult
 *
 * @author g30003063
 * @since 2022/5/20
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class CheckResult<T> {
    private ProtectedEnvironment environment;

    private ActionResult results;

    private T data;
}