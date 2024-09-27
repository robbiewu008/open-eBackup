package openbackup.system.base.sdk.dee.model;

import lombok.Data;

/**
 * 文件和文件目录信息
 *
 * @author jwx701567
 * @since 2021-12-17
 */
@Data
public class FineGrainedRestore {
    private String path;

    private String modifyTime;

    private Long size;

    private String type;

    private Boolean hasChildren;

    private String extendInfo;

    private String resType;

    private RestoreFilesResponse children;
}