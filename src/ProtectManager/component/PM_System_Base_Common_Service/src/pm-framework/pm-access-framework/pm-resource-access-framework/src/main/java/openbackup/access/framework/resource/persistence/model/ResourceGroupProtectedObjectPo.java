package openbackup.access.framework.resource.persistence.model;

import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Getter;
import lombok.Setter;

/**
 * ResourceGroup Member Po
 *
 * @author c00631681
 * @since 2024-1-18
 */
@TableName("T_RESOURCE_GROUP_MEMBER")
@Getter
@Setter
public class ResourceGroupProtectedObjectPo {
    /**
     * UUID
     */
    @TableId
    private String uuid;

    /**
     * 资源ID
     */
    private String sourceId;

    /**
     * 资源子类
     */
    private String sourceSubType;

    /**
     * 资源组id
     */
    private String resourceGroupId;
}