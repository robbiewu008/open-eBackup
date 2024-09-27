package openbackup.access.framework.resource.client.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 设备更新状态响应
 *
 * @author c00441246
 * @since 2023-10-14
 */
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
public class DeviceUpdateStatusResponse {
    /**
     * 设备更新状态
     */
    @JsonProperty("deviceUpdateStatus")
    private DeviceUpdateStatus deviceUpdateStatus;
}
