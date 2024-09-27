package openbackup.system.base.sdk.restore.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 内部接口调用恢复接口参数
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-07-26
 */
@Data
public class RestoreRequestInternal {
    /**
     * user_id 用户ID
     */
    @JsonProperty("user_id")
    private String userId;

    /**
     * restore_req_string 前端下发的恢复任务参数字符串
     */
    @JsonProperty("restore_req_string")
    private String requestReq;
}
