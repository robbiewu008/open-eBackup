package openbackup.cnware.protection.access.provider.indexer;

import openbackup.cnware.protection.access.dto.CnwareVolInfo;
import openbackup.data.access.framework.copy.index.provider.AbstractVmIndexerProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.index.IndexerProvider;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.security.EncryptorUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.storage.StorageRestClient;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * CnwareIndexerProvider CNware索引
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-03-09
 */
@Component
@Slf4j
public class CnwareIndexerProvider extends AbstractVmIndexerProvider implements IndexerProvider {
    /**
     * 构造函数注入
     *
     * @param storageRestClient storageRestClient
     * @param copyRestApi copyRestApi
     * @param notifyManager notifyManager
     * @param encryptorUtil encryptorUtil
     */
    public CnwareIndexerProvider(StorageRestClient storageRestClient, CopyRestApi copyRestApi,
        NotifyManager notifyManager, EncryptorUtil encryptorUtil) {
        super(storageRestClient, copyRestApi, notifyManager, encryptorUtil);
    }

    /**
     * provider过滤器，过滤条件接口
     *
     * @param resourceSubType 资源子类型
     * @return 检测结果true or false
     */
    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.CNWARE_VM.equalsSubType(resourceSubType);
    }

    /**
     * 生成索引文件
     *
     * @param requestId 请求id
     * @param copy 副本信息
     * @return 是否成功
     */
    @Override
    public boolean generateIndexFile(String requestId, CopyBo copy) {
        sendScanRequest(requestId, copy);
        return true;
    }

    /**
     * 生成快照元数据信息
     *
     * @param properties 配置信息
     * @return 元数据信息
     */
    @Override
    protected String buildSnapMetadata(JSONObject properties) {
        return CnwareVolInfo.convert2IndexDiskInfos(properties);
    }
}
