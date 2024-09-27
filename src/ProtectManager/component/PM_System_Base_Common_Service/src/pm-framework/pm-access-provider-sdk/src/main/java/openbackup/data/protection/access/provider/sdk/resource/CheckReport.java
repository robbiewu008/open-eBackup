/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

/**
 * The CheckReport
 *
 * @author g30003063
 * @since 2022-05-20
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class CheckReport<T> {
    private ProtectedResource resource;

    private List<CheckResult<T>> results;
}