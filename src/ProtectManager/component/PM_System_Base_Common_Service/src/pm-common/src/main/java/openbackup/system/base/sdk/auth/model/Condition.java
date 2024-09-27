/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * Condition
 *
 * @author z00664377
 * @since 2023-08-09
 */
@Getter
@Setter
public class Condition {
    private List<Constraint> constraint;
}
