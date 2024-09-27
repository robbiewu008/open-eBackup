package openbackup.data.protection.access.provider.sdk.resourcegroup.req;

import openbackup.system.base.common.utils.JSONObject;

import lombok.Data;

/**
 * 保护中间业务实体
 *
 * @author z30062305
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-07-29
 */

@Data
public class ProtectedObjectRequest {
    private String resourceGroupId;

    private String slaId;

    private JSONObject extParams;
}
