/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.storage.model;

import lombok.Data;

/**
 * 存储库基类
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-06-19
 */
@Data
public class RepositoryBase {
    private String repositoryId;

    private String name;

    private Long totalSize;

    private Long usedSize;

    private Long freeSize;

    private Integer type;

    private Integer status;

    private Float alarmThreashold;
}
