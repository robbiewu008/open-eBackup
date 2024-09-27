package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;

/**
 * 增加copy archive map映射请求
 *
 * @author g00500588
 * @since 2021/12/11
 */
@AllArgsConstructor
@Data
public class AddCopyArchiveMapRequest {
    @JsonProperty("copy_id")
    private String copyId;

    @JsonProperty("storage_id")
    private String storageId;

    @JsonProperty("resource_id")
    private String resourceId;
}
