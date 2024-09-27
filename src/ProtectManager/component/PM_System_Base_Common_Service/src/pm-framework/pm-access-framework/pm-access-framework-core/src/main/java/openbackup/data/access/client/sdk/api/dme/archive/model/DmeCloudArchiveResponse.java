package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * DME Clould Archive Copy Delete Common Response
 *
 * @author d00512967
 * @since 2020-12-14
 */
@Data
public class DmeCloudArchiveResponse {
    @JsonProperty("Id")
    private String id;

    @JsonProperty("TaskId")
    private String taskId;
}
