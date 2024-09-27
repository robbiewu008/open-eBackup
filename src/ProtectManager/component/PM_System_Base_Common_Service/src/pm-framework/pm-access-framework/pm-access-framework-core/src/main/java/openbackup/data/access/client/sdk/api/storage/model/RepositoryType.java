/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.storage.model;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-06-29
 */
public enum RepositoryType {
    /**
     * 本地存储库类型
     */
    LOCAL(1),
    /**
     * S3存储库类型
     */
    S3(2),
    /**
     * 蓝光存储库类型
     */
    BLUE_RAY(3),
    /**
     * 磁带存储库类型
     */
    TAPE(4);

    private int type;

    RepositoryType(int type) {
        this.type = type;
    }

    public int getType() {
        return type;
    }
}
