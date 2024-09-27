package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 扫描归档副本信息列表 copy详情
 *
 * @author z30009433
 * @since 2020-12-31
 */
@Data
public class CopyItem {
    @JsonProperty("PMMetaData")
    String copyInfo;

    @JsonProperty("DMECopyID")
    String copyId;
}
