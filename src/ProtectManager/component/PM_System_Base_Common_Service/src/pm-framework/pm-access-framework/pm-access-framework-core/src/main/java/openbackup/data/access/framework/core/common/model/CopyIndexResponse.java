package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Global Search index
 *
 * @author l00347293
 * @since 2021-01-06
 **/
@Data
public class CopyIndexResponse {
    @JsonProperty("copy_id")
    private String copyId;

    @JsonProperty("request_id")
    private String requestId;

    @JsonProperty("gen_index")
    private String genIndex;
}
