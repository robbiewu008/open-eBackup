package openbackup.system.base.sdk.copy.model;

import openbackup.system.base.common.utils.JSONObject;

import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Max;

/**
 * Copy Replication Import Param
 *
 * @author l00272247
 * @since 2020-12-15
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class CopyReplicationImportParam {
    private static final int MAX_METADATA_LEN = 30000;

    @Length(max = MAX_METADATA_LEN)
    private String metadata;

    @Max(Long.MAX_VALUE)
    private long timestamp;

    @Max(Long.MAX_VALUE)
    private long generatedTime;

    private JSONObject properties;
}
