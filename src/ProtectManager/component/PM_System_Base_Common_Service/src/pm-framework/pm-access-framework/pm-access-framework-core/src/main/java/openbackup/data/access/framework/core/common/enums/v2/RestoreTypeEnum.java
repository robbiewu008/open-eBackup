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
package openbackup.data.access.framework.core.common.enums.v2;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import java.util.Arrays;

/**
 * 恢复任务类型枚举定义
 *
 */
public enum RestoreTypeEnum {
    /**
     * 普通恢复
     */
    CR("normalRestore", "恢复", "Restore"),
    /**
     * 即时恢复
     */
    IR("instantRestore", "即时恢复", "Instant Restore"),
    /**
     * 细粒度恢复
     */
    FLR("fineGrainedRestore", "文件级恢复", "File-level Restoration");
    private final String type;

    private final String nameCn;

    private final String nameEn;

    RestoreTypeEnum(String type, String nameCn, String nameEn) {
        this.type = type;
        this.nameCn = nameCn;
        this.nameEn = nameEn;
    }

    @JsonValue
    public String getType() {
        return type;
    }

    /**
     * get nameCn
     *
     * @return string
     */
    public String getNameCn() {
        return nameCn;
    }

    /**
     * get nameEn
     *
     * @return string
     */
    public String getNameEn() {
        return nameEn;
    }

    /**
     * 根据恢复类型获取恢复类型的枚举类
     *
     * @param type 恢复类型
     * @return 恢复任务类型 {@code RestoreTypeEnum}
     */
    @JsonCreator
    public static RestoreTypeEnum getByType(String type) {
        return Arrays.stream(RestoreTypeEnum.values())
            .filter(restoreType -> restoreType.type.equals(type))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
