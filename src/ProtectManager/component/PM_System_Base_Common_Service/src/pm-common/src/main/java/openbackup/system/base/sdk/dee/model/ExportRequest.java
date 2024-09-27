package openbackup.system.base.sdk.dee.model;

import lombok.Data;

import java.util.List;

/**
 * 副本相关的接口
 *
 * @author jwx701567
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-01-28
 */
@Data
public class ExportRequest {
    /**
     * 下载路径
     */
    private List<String> paths;

    /**
     * 请求id
     */
    private String requestId;

    /**
     * 副本信息
     */
    private DownLoadCopyInfo copyInfo;
}

