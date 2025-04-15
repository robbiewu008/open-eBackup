/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.sdk.job.model;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;
import com.google.common.collect.ImmutableList;

import openbackup.system.base.util.EnumUtil;

import java.util.List;

/**
 * 功能描述
 *
 */
public enum JobLogLevelEnum {
    /**
     * INFO
     */
    INFO("info"),

    /**
     * WARNING
     */
    WARNING("warning"),

    /**
     * ERROR
     */
    ERROR("error"),

    /**
     * fatal
     */
    FATAL("fatal");

    private final String type;

    /**
     * 需要展示任务事件的事件级别
     */
    public static final List<String> needShowLevels = ImmutableList.of(JobLogLevelEnum.WARNING.getValue(),
            JobLogLevelEnum.ERROR.getValue(), JobLogLevelEnum.FATAL.getValue());

    JobLogLevelEnum(String value) {
        this.type = value;
    }

    /**
     * get job type enum by str
     *
     * @param str str
     * @return job type enum
     */
    @JsonCreator
    public static JobLogLevelEnum get(String str) {
        return EnumUtil.get(JobLogLevelEnum.class, JobLogLevelEnum::getValue, str, false);
    }

    /**
     * get json value
     *
     * @return json value
     */
    @JsonValue
    public String getValue() {
        return type;
    }
}
