/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 调用hcs接口，根据条件查询浮动ip信息响应体
 *
 * @author z00664377
 * @since 2023-08-04
 */
@Getter
@Setter
public class QueryIpResponse {
    private List<ResponseObj> objList;
    private Long totalNum;
    private Long pageSize;
    private Long totalPageNo;
    private Long currentPage;
}
