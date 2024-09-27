package openbackup.system.base.sdk.functionswith;

/**
 * 删除用户功能开关服务（内部调用）
 *
 * @author w30042425
 * @since 2023-01-18
 */
public interface InternalFunctionSwitchService {
    /**
     * 删除指定用户某一功能开关开启情况
     *
     * @param userId 用户ID
     * @return 删除用户功能表的行数
     */
    int delUserFunction(String userId);

    /**
     * 设置指定用户各功能开关开启情况
     *
     * @param userId 用户ID
     * @param canBackUp 备份功能开关开启情况
     * @param canRestore 恢复功能开关开启情况
     * @param canArchive 归档功能开关开启情况
     * @param canReplication 复制功能开关开启情况
     * @return 返回添加/修改数据行数
     */
    String setUserFunction(String userId, boolean canBackUp, boolean canRestore, boolean canArchive,
        boolean canReplication);
}
