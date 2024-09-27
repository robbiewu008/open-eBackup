package openbackup.data.access.client.sdk.api.framework.dee.model;

import lombok.Data;

import java.util.List;

/**
 * 一体机共享路径恢复创建共享请求
 *
 * @author w00574036
 * @since 2024-04-19
 * @version [OceanCyber 300 1.2.0]
 */
@Data
public class OcLiveMountFsShareReq {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 任务id
     */
    private String taskId;

    /**
     * nfs及cifs共享信息
     */
    private List<OcLiveMountFsShareInfo> liveMountFsShareInfos;
}
