package openbackup.data.access.framework.protection.service.repository.strategies;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;

/**
 * 存储库的策略接口类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/8
 **/
public interface RepositoryStrategy {
    /**
     * 获取最新的认证信息
     *
     * @param repositoryId 存储库id
     * @return 最新的认证信息 {@code Authentication}
     */
    Authentication getAuthentication(String repositoryId);

    /**
     * 获取存储库最新的Endpoint信息
     *
     * @param repositoryId 存储库id
     * @return 连接信息 {@code Endpoint}
     */
    Endpoint getEndpoint(String repositoryId);

    /**
     * 获取完整的存储库的信息
     *
     * @param storageRepository 存储库基本信息
     * @return 完整存储库信息 {@code StorageRepository}
     */
    StorageRepository getRepository(BaseStorageRepository storageRepository);
}
