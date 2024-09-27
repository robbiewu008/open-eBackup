/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * QueryRegionAndProjectResponse
 *
 * @author y30021475
 * @since 2023-09-13
 */
@Getter
@Setter
public class QueryRegionAndProjectResponse {
    private int total;

    private List<HcsProject> projects;
}
