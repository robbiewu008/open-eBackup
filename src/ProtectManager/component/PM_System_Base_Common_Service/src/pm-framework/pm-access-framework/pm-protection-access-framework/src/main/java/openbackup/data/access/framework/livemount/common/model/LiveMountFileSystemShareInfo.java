package openbackup.data.access.framework.livemount.common.model;

import lombok.Data;

import java.util.Map;

/**
 * Live Mount File System Share Info
 *
 * @author l00272247
 * @since 2022-01-05
 */
@Data
public class LiveMountFileSystemShareInfo {
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
