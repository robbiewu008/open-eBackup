package openbackup.data.access.framework.copy.index.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 删除副本索引的响应
 *
 * @author g30003063
 * @since 2021-12-16
 */
@Getter
@Setter
public class CopyIndexDeleteResponseDto {
    @JsonProperty("request_id")
    private String requestId;

    @JsonProperty("user_id")
    private String userId;

    @JsonProperty("resource_id")
    private String resourceId;

    @JsonProperty("copy_id")
    private String copyId;

    @JsonProperty("status")
    private String status;

    @JsonProperty("error_code")
    private String errorCode;

    @JsonProperty("error_desc")
    private String errorDesc;
}
