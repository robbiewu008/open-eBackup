package openbackup.data.protection.access.provider.sdk.resource;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述 租户信息DTO
 *
 * @author s30031954
 * @since 2022-12-21
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class TenantInfo {
    // 租户ID
    private String tenantId;

    // 租户名称
    private String tenantName;

    // 设备ID
    private String deviceId;

    // 设备名称
    private String deviceName;
}