package openbackup.data.protection.access.provider.sdk.resource.model;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import lombok.Getter;
import lombok.Setter;

/**
 * 描述
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-22
 */
@Getter
@Setter
public class ResourceScanDto {
    private ProtectedResource resource;

    private Exception exception;
}
