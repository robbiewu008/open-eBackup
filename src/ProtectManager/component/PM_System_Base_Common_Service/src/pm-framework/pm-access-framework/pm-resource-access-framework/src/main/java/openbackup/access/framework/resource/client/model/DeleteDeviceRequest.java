package openbackup.access.framework.resource.client.model;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 删除设备请求体
 *
 * @author s30031954
 * @since 2022-12-29
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class DeleteDeviceRequest {
    /**
     * 设备ID
     */
    private String deviceId;
}
