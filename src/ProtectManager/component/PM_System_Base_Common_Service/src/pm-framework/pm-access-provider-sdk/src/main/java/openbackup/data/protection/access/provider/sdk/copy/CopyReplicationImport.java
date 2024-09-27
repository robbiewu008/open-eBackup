package openbackup.data.protection.access.provider.sdk.copy;

import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import org.hibernate.validator.constraints.Length;

import java.util.Map;

import javax.validation.constraints.Max;

/**
 * Copy Replication Import Param
 *
 * @author l00272247
 * @since 2020-12-15
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class CopyReplicationImport {
    private static final int MAX_METADATA_LEN = 100 * 1024;

    @Length(max = MAX_METADATA_LEN)
    private String metadata;

    @Max(Long.MAX_VALUE)
    private long timestamp;

    @Max(Long.MAX_VALUE)
    private long generatedTime;

    private Map properties;

    private ReplicationOriginCopyDuration originCopyDuration;

    // 存储的esn
    private String esn;

    // op的esn
    private String backupSoftwareEsn;
}
