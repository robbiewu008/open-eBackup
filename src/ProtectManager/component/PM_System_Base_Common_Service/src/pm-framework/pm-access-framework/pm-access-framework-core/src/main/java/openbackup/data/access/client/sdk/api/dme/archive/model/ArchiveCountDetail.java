package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 扫描归档副本总数 返回体详情
 *
 * @author z30009433
 * @since 2020-12-31
 */
@Data
public class ArchiveCountDetail {
    @JsonProperty("Count")
    private int count;

    @JsonProperty("TaskId")
    private String taskId;
}
