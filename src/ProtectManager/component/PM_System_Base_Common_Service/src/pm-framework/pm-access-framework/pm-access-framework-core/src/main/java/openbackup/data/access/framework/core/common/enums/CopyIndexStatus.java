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
 */
@Getter
public enum CopyIndexStatus {
    /**
     * 未索引
     */
    UNINDEXED("Unindexed"),

    /**
     * 索引中
     */
    INDEXING("Indexing"),

    /**
     * 已索引
     */
    INDEXED("Indexed"),

    /**
     * 索引失败
     */
    INDEX_FAIL("Index_fail"),

    /**
     * 索引删除失败
     */
    INDEX_DELETING("Index_deleting"),

    /**
     * 索引删除失败
     */
    INDEX_DELETE_FAIL("Index_delete_fail"),

    /**
     * 不支持索引
     */
    UNSUPPORT("Unsupport"),

    /**
     * index_scan_response_error_label
     */
    INDEX_SCAN_RESPONSE_ERROR_LABEL("index_scan_response_error_label"),

    /**
     * index_response_error_label
     */
    INDEX_RESPONSE_ERROR_LABEL("index_response_error_label"),

    /**
     * index_copy_status_error_label
     */
    INDEX_COPY_STATUS_ERROR_LABEL("index_copy_status_error_label");


    private String status;

    CopyIndexStatus(String status) {
        this.status = status;
    }

    /**
     * 获取副本索引状态
     *
     * @return string
     */
    @JsonValue
    public String getIndexStaus() {
        return status;
    }
}
