package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * VmwareFLRRequestBody
 *
 * @author p00511147
 * @since 2020-12-29
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class VmFlrRequest {
    private String jobId;

    private String requestId;

    private String defaultPublishTopic;

    private String responseTopic;

    private String userId;

    private String snapMetadata;

    private String snapId;

    private RestoreStorageInfo storageInfo;

    private List<String> paths;

    private VmFlrDestInfo destInfo;

    private String replaceMode;

    private String snapType;

    private String recordId;

    private String resourceSubType;

    /**
     * 设备Id
     */
    private String deviceId;

    /**
     * 存储单元id
     */
    private String storageId;
}
