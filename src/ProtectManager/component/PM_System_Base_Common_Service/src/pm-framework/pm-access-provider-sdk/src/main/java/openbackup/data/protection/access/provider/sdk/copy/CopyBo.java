package openbackup.data.protection.access.provider.sdk.copy;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

/**
 * 备份信息模型
 *
 * @author t00482481
 * @since 2020-07-02
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class CopyBo extends CopyInfoBo {
    @JsonProperty("uuid")
    private String uuid;

    @JsonProperty("amount")
    private int amount;

    @JsonProperty("gn")
    private int gn;

    @JsonProperty("prev_copy_id")
    private String prevCopyId;

    @JsonProperty("next_copy_id")
    private String nextCopyId;

    @JsonProperty("prev_copy_gn")
    private int prevCopyGn;

    @JsonProperty("next_copy_gn")
    private int nextCopyGn;
}
