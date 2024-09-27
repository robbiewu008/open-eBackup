package openbackup.data.protection.access.provider.sdk.protection.model;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import lombok.Getter;
import lombok.Setter;

/**
 * 用于project object provider的dto
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023/11/10
 */
@Getter
@Setter
public class CheckProtectObjectDto extends ProtectObjectReqBase {
    private ProtectedResource protectedResource;
}
