/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 向cmdb查询云服务参数返回体
 *
 * @author y30021475
 * @since 2023-07-27
 */
@Getter
@Setter
public class QueryCloudServiceParamsPageResponse {
    private int totalNum;

    private int pageSize;

    private int totalPageNo;

    private int currentPage;

    private List<CloudServiceParams> objList;
}
