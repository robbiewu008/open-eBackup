package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 复制副本记录表
 *
 * @author z30027603
 * @since 2022/11/23 16:58
 */
@Data
public class ReplicatedCopies {
    @JsonProperty("copy_id")
    private String copyId;

    @JsonProperty("resource_id")
    private String resourceId;

    @JsonProperty("esn")
    private String esn;
}
