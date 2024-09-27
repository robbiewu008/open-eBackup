package openbackup.data.access.client.sdk.api.framework.dme.replicate.model;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 接收任务行为请求对象
 *
 * @author y30037959
 * @since 2023-01-31
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class ReceiveJobActionRequest {
    /**
     * 0:开始接收复制任务 1:停止接收复制任务
     */
    private int action;
}
