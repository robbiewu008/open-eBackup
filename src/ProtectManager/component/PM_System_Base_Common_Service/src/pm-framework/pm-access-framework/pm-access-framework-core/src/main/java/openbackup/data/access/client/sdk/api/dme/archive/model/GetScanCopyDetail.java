package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 扫描归档副本信息列表 详情
 *
 * @author z30009433
 * @since 2020-12-31
 */
@Data
public class GetScanCopyDetail {
    @JsonProperty("CopyItems")
    List<CopyItem> copyItems;

    @JsonProperty("TaskId")
    private String taskId;
}
