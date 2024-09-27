/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.model.repository.tape;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import lombok.AllArgsConstructor;

/**
 * 介质集类型
 *
 * @author z30006621
 * @since 2021-08-24
 */
@AllArgsConstructor
public enum MediaSetAppType {
    /**
     * ORACLE
     */
    ORACLE(1),

    /**
     * VMWARE
     */
    VMWARE(2),

    /**
     * DORADO_FILE_SYSTEM
     */
    DORADO_FILE_SYSTEM(3),

    /**
     * NAS_SHAR
     */
    NAS_SHAR(4),

    /**
     * KUBERNETES_MYSQL
     */
    KUBERNETES_MYSQL(5),

    /**
     * KUBERNETES_COMMON
     */
    KUBERNETES_COMMON(6),

    /**
     * DUPLICATION_COPY
     */
    DUPLICATION_COPY(7),

    /**
     * IMPORT_COPY
     */
    IMPORT_COPY(8),

    /**
     * HDFS
     */
    HDFS(9),

    /**
     * HBASE
     */
    HBASE(10);

    private final int type;

    @JsonValue
    public int getType() {
        return type;
    }

    /**
     * 根据传入的值获取对应的枚举值
     *
     * @param value value
     * @return MediaSetType
     */
    @JsonCreator(mode = JsonCreator.Mode.DELEGATING)
    public static MediaSetAppType getValue(int value) {
        for (MediaSetAppType mediaSetAppType : MediaSetAppType.values()) {
            if (value == mediaSetAppType.type) {
                return mediaSetAppType;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
    }
}
