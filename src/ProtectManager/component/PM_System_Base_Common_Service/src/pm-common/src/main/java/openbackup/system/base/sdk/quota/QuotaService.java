package openbackup.system.base.sdk.quota;

/**
 * 删除用户的配额信息
 *
 * @author w30042425
 * @since 2023-01-18
 */
public interface QuotaService {
    /**
     * 根据userId,resourceId删除额度记录
     *
     * @param userId 用户id
     * @param resourceId 资源id
     */
    void deleteUserQuotaByUserIdAndResourceId(String userId, String resourceId);

    /**
     * 根据userId初始化hcs用户额度记录
     *
     * @param userId 用户id
     */
    void initHcsUserQuota(String userId);
}
