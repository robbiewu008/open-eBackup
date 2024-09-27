/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.constants;

import lombok.Getter;
import lombok.Setter;

/**
 * 双活pairBO
 *
 * @author mwx776342
 * @since 2022/03/17
 */
@Getter
@Setter
public class HyperMetroPair {
    /**
     * ID
     */
    private String id;

    /**
     * 判断 是否是主端(isPrimary, true：是主端 ，false：不是主端)
     */
    private boolean isPrimary;
}
