/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.servitization.controller.req;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotBlank;

/**
 * VpcRequest
 *
 * @author l30044826
 * @since 2023-08-11
 */
@Getter
@Setter
public class VpcRequest {
    @JsonProperty("Vpcid")
    @NotBlank
    @Length(min = 1, max = 128)
    private String vpcId;

    @JsonProperty("Projectid")
    @NotBlank
    @Length(min = 1, max = 128)
    private String projectId;
}
