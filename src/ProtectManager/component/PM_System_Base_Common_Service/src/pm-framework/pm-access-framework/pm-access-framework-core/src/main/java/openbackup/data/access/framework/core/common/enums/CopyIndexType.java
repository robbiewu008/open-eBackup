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
package openbackup.data.access.framework.core.common.enums;

import com.fasterxml.jackson.annotation.JsonValue;

import lombok.Getter;

/**
 * Copy index status
 *
 * @author l00347293
 * @since 2021-01-14
 */
@Getter
public enum CopyIndexType {
    /**
     * 全量索引
     */
    FULL("Full"),

    /**
     * 增量索引
     */
    INCREMENTAL("Incremental");
    private String indexType;

    CopyIndexType(String indexType) {
        this.indexType = indexType;
    }

    /**
     * 获取副本索引类型
     *
     * @return string
     */
    @JsonValue
    public String getIndexType() {
        return indexType;
    }
}
