/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dee.model;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

/**
 * 删除副本索引请求
 *
 * @author z00842230
 * @version [oceanprotect databackup 1.7.0]
 * @since 2024-08-08
 */
@AllArgsConstructor
@NoArgsConstructor
@Getter
@Setter
public class DeleteCopyIndexRequest {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 资源id
     */
    private String resourceId;

    /**
     * 副本id列表
     */
    private List<String> copyIdList;

    /**
     * 副本链id
     */
    private String chainId;

    /**
     * 用户id
     */
    private String userId;
}