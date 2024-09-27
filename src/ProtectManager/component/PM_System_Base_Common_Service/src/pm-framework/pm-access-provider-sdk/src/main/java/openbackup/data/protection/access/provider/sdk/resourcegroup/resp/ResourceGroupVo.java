package openbackup.data.protection.access.provider.sdk.resourcegroup.resp;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 资源组列表查询返回体
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-25
 */

@Getter
@Setter
public class ResourceGroupVo {
    private String uuid;

    private String name;

    private String path;

    private int resourceCount;

    private String sourceType;

    private int protectionStatus;

    private ResourceGroupProtectedObjectVo protectedObject;

    private String sourceSubType;

    private String createdTime;

    private String userId;

    private List<ResourceGroupMemberVo> resourceGroupMembers;
}