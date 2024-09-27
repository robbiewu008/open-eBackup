/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.Set;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-11-13
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class NfsProtocol {
    private String sharePath;

    private Set<String> whitelist;
}
