/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.kafka;

/**
 * Message Phase
 *
 * @author l00272247
 * @since 2020-10-20
 */
public enum MessagePhase {
    /**
     * FAILURE
     */
    FAILURE,
    /**
     * SUCCESS
     */
    SUCCESS,
    /**
     * COMPLETE
     */
    COMPLETE
}
