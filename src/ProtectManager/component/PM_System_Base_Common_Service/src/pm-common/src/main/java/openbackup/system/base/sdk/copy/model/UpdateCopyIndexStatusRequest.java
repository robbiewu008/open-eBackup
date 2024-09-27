/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

/**
 * 更新副本索引状态请求
 *
 * @author z00842230
 * @version [oceanprotect databackup 1.7.0]
 * @since 2024-08-14
 */
@AllArgsConstructor
@NoArgsConstructor
@Getter
@Setter
public class UpdateCopyIndexStatusRequest {
    /**
     * 副本id列表
     */
    @JsonProperty("copy_id_list")
    List<String> copyIdList;

    /**
     * 待更新索引状态
     */
    @JsonProperty("index_status")
    String indexStatus;

    /**
     * 错误码
     */
    @JsonProperty("error_code")
    String errorCode;
}