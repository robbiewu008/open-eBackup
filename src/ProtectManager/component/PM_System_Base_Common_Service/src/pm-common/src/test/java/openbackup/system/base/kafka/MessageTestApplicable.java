/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.kafka;

import openbackup.system.base.util.Applicable;

/**
 * Message Test Applicable
 *
 * @author l00272247
 * @since 2022-02-26
 */
public class MessageTestApplicable implements Applicable<String> {
    /**
     * applicable
     *
     * @param object object
     * @return null
     */
    @Override
    public boolean applicable(String object) {
        return false;
    }
}
