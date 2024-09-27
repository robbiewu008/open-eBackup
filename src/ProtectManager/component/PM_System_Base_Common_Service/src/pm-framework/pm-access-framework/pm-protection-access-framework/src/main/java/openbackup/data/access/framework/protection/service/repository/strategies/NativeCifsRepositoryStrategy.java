package openbackup.data.access.framework.protection.service.repository.strategies;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.session.IStorageDeviceRepository;

import org.springframework.stereotype.Component;

/**
 * 本地存储CIFS库协议的策略类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/8
 **/
@Component("nativeCifsRepositoryStrategy")
public class NativeCifsRepositoryStrategy extends BaseNativeRepositoryStrategy implements RepositoryStrategy {
    public NativeCifsRepositoryStrategy(ClusterNativeApi clusterNativeApi, IStorageDeviceRepository repository) {
        super(clusterNativeApi, repository);
    }

    @Override
    public Authentication getAuthentication(String repositoryId) {
        // 本地存储 用户名/密码字段进行认证
        return super.buildAuthentication();
    }

    @Override
    public Endpoint getEndpoint(String repositoryId) {
        return super.buildEndPoint(repositoryId);
    }

    @Override
    public StorageRepository getRepository(BaseStorageRepository baseRepository) {
        final StorageRepository nativeRepository = super.getNativeRepository(baseRepository);
        nativeRepository.setProtocol(RepositoryProtocolEnum.CIFS.getProtocol());
        return nativeRepository;
    }
}
