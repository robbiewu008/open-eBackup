/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
