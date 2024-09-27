package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 归档副本请求
 *
 * @author y00490893
 * @since 2020-12-16
 */
@Data
public class ArchiveDetail {
    @JsonProperty("Id")
    private String id;

    @JsonProperty("TaskId")
    private String taskId;
}
