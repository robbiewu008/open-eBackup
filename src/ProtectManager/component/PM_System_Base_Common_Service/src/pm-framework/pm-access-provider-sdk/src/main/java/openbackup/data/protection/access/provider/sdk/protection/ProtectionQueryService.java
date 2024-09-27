package openbackup.data.protection.access.provider.sdk.protection;

/**
 * 查询关联资源信息
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-06-19
 */
public interface ProtectionQueryService {
    /**
     * 根据SLA的ID和userId查询关联资源数量和subType
     *
     * @param slaName SLA的Name
     * @param userId 用户Id
     * @param subType 资源子类型
     * @return 返回数量
     */
    int countBySubTypeAndSlaName(String slaName, String userId, String subType);
}
