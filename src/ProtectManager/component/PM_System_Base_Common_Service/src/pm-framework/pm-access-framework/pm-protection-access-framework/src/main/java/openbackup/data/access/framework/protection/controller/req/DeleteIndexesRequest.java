/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.controller.req;

import lombok.Getter;
import lombok.Setter;

/**
 * The DeleteIndexesRequest
 *
 * @author g30003063
 * @since 2021/12/16
 */
@Getter
@Setter
public class DeleteIndexesRequest {
    private String requestId;

    private String resourceId;
}
