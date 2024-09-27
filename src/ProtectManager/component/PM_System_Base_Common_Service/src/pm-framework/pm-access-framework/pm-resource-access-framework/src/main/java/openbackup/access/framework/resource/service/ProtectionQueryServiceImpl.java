package openbackup.access.framework.resource.service;

import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.protection.access.provider.sdk.protection.ProtectionQueryService;

import org.springframework.stereotype.Service;

/**
 * 查询关联资源信息
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-06-19
 */
@Service
public class ProtectionQueryServiceImpl implements ProtectionQueryService {
    private final ProtectedObjectMapper protectedObjectMapper;

    public ProtectionQueryServiceImpl(ProtectedObjectMapper protectedObjectMapper) {
        this.protectedObjectMapper = protectedObjectMapper;
    }

    /**
     * 根据SLA的ID和userId查询关联资源数量和subType
     *
     * @param slaName SLA的Name
     * @param userId 用户Id
     * @param subType 资源子类型
     * @return 返回数量
     */
    public int countBySubTypeAndSlaName(String slaName, String userId, String subType) {
        return protectedObjectMapper.countBySubTypeAndSlaName(slaName, userId, subType);
    }
}
