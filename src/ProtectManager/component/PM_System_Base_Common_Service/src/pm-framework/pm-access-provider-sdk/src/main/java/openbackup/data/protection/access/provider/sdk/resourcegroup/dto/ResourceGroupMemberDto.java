package openbackup.data.protection.access.provider.sdk.resourcegroup.dto;

import lombok.Getter;
import lombok.Setter;

/**
 * ResourceGroupMemberDto
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-26
 */
@Getter
@Setter
public class ResourceGroupMemberDto {
    private String uuid;

    private String sourceId;

    private String sourceSubType;

    private String resourceGroupId;
}