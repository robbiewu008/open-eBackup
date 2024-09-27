package openbackup.data.access.client.sdk.api.framework.dme;

import lombok.Data;

import java.util.Map;

/**
 * Dme File System Share Info
 *
 * @author l00272247
 * @since 2022-01-06
 */
@Data
public class DmeFileSystemShareInfo {
    /**
     * 0: nfs; 1: cifs
     */
    private int type;
    private String fileSystemName;

    /**
     * 0:只读; 1:读写
     */
    private int accessPermission;
    private Map<String, Object> advanceParams;
}
