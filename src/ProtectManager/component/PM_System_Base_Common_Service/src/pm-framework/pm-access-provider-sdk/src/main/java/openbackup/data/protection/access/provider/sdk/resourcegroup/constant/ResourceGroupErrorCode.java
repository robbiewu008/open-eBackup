package openbackup.data.protection.access.provider.sdk.resourcegroup.constant;

/**
 * 错误码
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-22
 */
public class ResourceGroupErrorCode {
    /**
     * 超出资源组规格
     * 原因：特定资源类型的资源组数量超出了规格。
     * 建议：删除同类型其他无用资源组后重试
     */
    public static final Long EXCEED_RESOURCE_GROUP_MAX_NUM_LIMIT = 0x640333BFL;

    /**
     * 超出资源组成员规格
     * 原因：资源组成员数量超出规格。
     * 建议：重新添加符合要求的资源组成员数量的成员后重试
     */
    public static final Long EXCEED_RESOURCE_GROUP_MEMBER_MAX_NUM_LIMIT = 0x640333A2L;

    /**
     * 资源组成员不能添加到多个组
     * 原因：通过资源组成员已添加到其他组
     * 建议：重新添加符合要求的资源组成员后重试
     */
    public static final Long RESOURCE_ID_EXIST_IN_OTHER_GROUP = 0x640333C3L;

    /**
     * 资源组成员类型与资源组类型不匹配
     * 原因：通过资源组成员类型与资源组类型不匹配
     * 建议：重新添加符合要求的资源组成员数量的成员后重试
     */
    public static final Long SOURCE_SUB_TYPE_NOT_MATCH = 0x640333C0L;

    /**
     * 资源组成员如果已保护则不能加入组中
     * 原因：通过资源组成员已创建保护，不能再加入到资源组中
     * 建议：重新添加符合要求的资源组成员数量的成员后重试
     */
    public static final Long RESOURCE_GROUP_MEMBER_PROTECT_STATUS_NOT_VALID = 0x640333C1L;
}