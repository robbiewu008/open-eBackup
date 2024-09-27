/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.ndmp.protection.access.constant;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述 NdmpSrc
 *
 * @author t30021437
 * @since 2023-05-15
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class NdmpSrc {
    /**
     * ndmpSrc ndmpServerIp
     */
    private String serverIp;

    /**
     * ndmpSrc port
     */
    private String port;

    /**
     * ndmpSrc path
     */
    private String path;

    /**
     * ndmpSrc status
     */
    private String status;
}