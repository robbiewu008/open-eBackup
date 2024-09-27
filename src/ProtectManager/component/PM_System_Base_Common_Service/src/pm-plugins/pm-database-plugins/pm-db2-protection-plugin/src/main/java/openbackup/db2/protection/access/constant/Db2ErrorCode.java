package openbackup.db2.protection.access.constant;

/**
 * db2错误码
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-05
 */
public class Db2ErrorCode {
    /**
     * 场景: 执行创建/修改保护对象操作时，由于所选保护对象子资源数量超过系统上限，操作失败。
     * 原因: 所选保护对象子资源数量超过系统上限（{0}）。
     * 建议: 请删除不再使用的保护对象子资源后重试。
     */
    public static final long CHECK_RESOURCES_SIZE_ERROR = 1577209985L;
}
