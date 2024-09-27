package openbackup.data.protection.access.provider.sdk.resource;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述 文件系统DTO
 *
 * @author s30031954
 * @since 2022-12-21
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class FileSystemInfo {
    // 租户ID
    private String tenantId;

    // 设备ID
    private String deviceId;

    // 文件系统UUID
    private String fileUuid;

    // 文件系统ID
    private String fileId;

    // 文件系统名称
    private String fileName;

    // 文件系统子类型
    private String subType;
}