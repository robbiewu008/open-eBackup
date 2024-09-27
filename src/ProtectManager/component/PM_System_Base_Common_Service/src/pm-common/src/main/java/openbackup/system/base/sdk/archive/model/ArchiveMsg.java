/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import org.hibernate.validator.constraints.Length;

import java.util.Map;

import javax.validation.constraints.NotNull;

/**
 * 备份完成后python发给主集群java的归档消息
 *
 * @author y30046482
 * @since 2023-07-26
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
@AllArgsConstructor
@NoArgsConstructor
public class ArchiveMsg {
    @NotNull
    @Length(max = 255)
    @JsonProperty("request_id")
    private String requestId;

    @NotNull
    @JsonProperty("params")
    private Map params;
}
