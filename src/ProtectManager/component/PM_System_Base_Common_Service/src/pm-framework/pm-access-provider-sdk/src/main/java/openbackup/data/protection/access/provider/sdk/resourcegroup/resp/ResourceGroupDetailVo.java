package openbackup.data.protection.access.provider.sdk.resourcegroup.resp;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 资源组详情查询返回体
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-25
 */

@Getter
@Setter
public class ResourceGroupDetailVo {
    private String uuid;

    private String name;

    private String path;

    private String sourceType;

    private String sourceSubType;

    private String createdTime;

    private int protectionStatus;

    private String userId;

    private ResourceGroupProtectedObjectVo protectedObject;

    private List<ResourceGroupMemberVo> resourceGroupMembers;
}