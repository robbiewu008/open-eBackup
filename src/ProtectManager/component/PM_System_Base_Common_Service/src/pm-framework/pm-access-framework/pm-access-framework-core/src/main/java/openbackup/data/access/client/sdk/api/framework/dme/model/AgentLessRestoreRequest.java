package openbackup.data.access.client.sdk.api.framework.dme.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 功能描述
 *
 * @author y30044273
 * @since 2023-12-01
 * @version [OceanProtect DataBackup 1.6.0]
 */
@Getter
@Setter
public class AgentLessRestoreRequest extends AgentLessActionRequest {
    private String srcCopyId;

    private String targetFilesystemName;
}
