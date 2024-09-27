package openbackup.gaussdbdws.protection.access.constant;

/**
 * 功能描述: DWS 错误码
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-23
 */
public class DwsErrorCode {
    /**
     * Dws资源已达到上限
     */
    public static final long DWS_RESOURCE_REACHED_THE_UPPER_LIMIT = 1677931389L;

    /**
     * 场景: 执行创建/修改保护对象操作时，由于所选保护对象子资源数量超过系统上限，操作失败。
     * 原因: 所选保护对象子资源数量超过系统上限（{0}）。
     * 建议: 请删除不再使用的保护对象子资源后重试。
     */
    public static final long CHECK_RESOURCES_SIZE_ERROR = 1577209985L;

    /**
     * 错误场景：执行修改SLA时，由于关联资源的类型不支持保护策略，操作失败。
     * 原因：关联资源的类型（{0}）不支持保护策略（{1}）。
     * 建议：请删除关联资源或者重新创建保护策略。
     */
    public static final long NO_SUPPORT_SLA = 1677933571L;
}