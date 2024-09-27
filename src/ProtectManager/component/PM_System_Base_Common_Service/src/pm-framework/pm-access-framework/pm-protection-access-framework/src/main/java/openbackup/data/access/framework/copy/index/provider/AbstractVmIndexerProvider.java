package openbackup.data.access.framework.copy.index.provider;

import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.data.access.framework.core.common.model.RestoreStorageInfo;
import openbackup.data.access.framework.core.common.model.ScanRequest;
import openbackup.data.access.framework.core.common.model.SnapInfo;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.security.EncryptorUtil;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.storage.StorageRestClient;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;

import java.util.Locale;
import java.util.Optional;

/**
 * 索引provider类
 *
 * @author y30037959
 * @since 2023-06-01
 */
@Slf4j
public abstract class AbstractVmIndexerProvider {
    private static final String DORADO = "DORADO";

    private static final String NAS_PROTOCOL = "NAS";

    private static final String DEFAULT_STORAGE_POOL = "0";

    /**
     * storageRestClient
     */
    protected final StorageRestClient storageRestClient;

    /**
     * copyRestApi
     */
    protected final CopyRestApi copyRestApi;

    /**
     * notifyManager
     */
    protected final NotifyManager notifyManager;

    /**
     * encryptorUtil
     */
    protected final EncryptorUtil encryptorUtil;

    /**
     * taskRepositoryManager
     */
    protected TaskRepositoryManager taskRepositoryManager;

    @Autowired
    private StorageUnitService storageUnitService;

    /**
     * 构造函数
     *
     * @param storageRestClient storageRestClient
     * @param copyRestApi copyRestApi
     * @param notifyManager notifyManager
     * @param encryptorUtil encryptorUtil
     */
    public AbstractVmIndexerProvider(StorageRestClient storageRestClient, CopyRestApi copyRestApi,
        NotifyManager notifyManager, EncryptorUtil encryptorUtil) {
        this.storageRestClient = storageRestClient;
        this.copyRestApi = copyRestApi;
        this.notifyManager = notifyManager;
        this.encryptorUtil = encryptorUtil;
    }

    @Autowired
    public void setTaskRepositoryManager(TaskRepositoryManager taskRepositoryManager) {
        this.taskRepositoryManager = taskRepositoryManager;
    }

    /**
     * 发送扫描请求
     *
     * @param requestId 请求id
     * @param copy 副本信息
     */
    protected final void sendScanRequest(String requestId, CopyBo copy) {
        copyRestApi.updateCopyIndexStatus(copy.getUuid(), CopyIndexStatus.INDEXING.getIndexStaus(), "");
        log.info("Update copy status as {} when scanning, copy id: {}", CopyIndexStatus.INDEXING.getIndexStaus(),
            copy.getUuid());
        try {
            ScanRequest request = buildScanRequest(requestId, copy);
            sendScanMessage(request);
            log.info("Send copy index scan message success, copy id: {}", copy.getUuid());
        } catch (Exception e) {
            log.error("Send copy index scan message fail, copy id: {}", copy.getUuid(),
                ExceptionUtil.getErrorMessage(e));
            String errorCode = CopyIndexStatus.INDEX_SCAN_RESPONSE_ERROR_LABEL.getIndexStaus();
            copyRestApi.updateCopyIndexStatus(copy.getUuid(), CopyIndexStatus.INDEX_FAIL.getIndexStaus(), errorCode);
            log.info("Update copy status as {} when scanning, copy id: {}", CopyIndexStatus.INDEX_FAIL.getIndexStaus(),
                copy.getUuid());
        }
    }

    private ScanRequest buildScanRequest(String requestId, CopyBo copy) {
        ScanRequest request = new ScanRequest();
            request.setRequestId(requestId);
            request.setDefaultPublishTopic(TopicConstants.SCAN_REQUEST);
            request.setResponseTopic(TopicConstants.SCAN_RESPONSE);

            SnapInfo snapInfo = new SnapInfo();
            snapInfo.setChainId(copy.getChainId());
            snapInfo.setGn(copy.getGn());
            snapInfo.setTimestamp(copy.getTimestamp());
            snapInfo.setResourceId(copy.getResourceId());
            snapInfo.setResourceName(copy.getResourceName());
            snapInfo.setResourceType(copy.getResourceSubType().toLowerCase(Locale.ENGLISH));
            snapInfo.setEsn(copy.getDeviceEsn());

            JSONObject properties = JSONObject.fromObject(copy.getProperties());
            snapInfo.setSnapMetadata(buildSnapMetadata(properties));
            snapInfo.setSnapId(copy.getUuid());
            snapInfo.setSnapType(copy.getGeneratedBy());
            snapInfo.setUserId(copy.getUserId());

            request.setSnapInfo(snapInfo);
            request.setStorageInfo(obtainStorageInfo(copy));

        request.setStorageRepository(
            taskRepositoryManager.getCopyRepoWithAuth(copy.getProperties(), copy.getStorageUnitId(),
                RepositoryTypeEnum.DATA.getType()).orElse(null));
        return request;
    }

    private RestoreStorageInfo obtainStorageInfo(CopyBo copy) {
        RestoreStorageInfo flrStorage = new RestoreStorageInfo();
        flrStorage.setStorageType(DORADO);
        flrStorage.setProtocol(NAS_PROTOCOL);

        // 下发DEE参数，适配软硬解耦
        Optional<StorageUnitVo> storageUnitVoOptional = storageUnitService.getStorageUnitById(copy.getStorageUnitId());
        if (storageUnitVoOptional.isPresent()) {
            StorageUnitVo storageUnitVo = storageUnitVoOptional.get();
            flrStorage.setStorageId(storageUnitVo.getPoolId());
            flrStorage.setDeviceEsn(storageUnitVo.getDeviceId());
        } else {
            // 取默认值
            flrStorage.setStorageId(DEFAULT_STORAGE_POOL);
            flrStorage.setDeviceEsn(copy.getDeviceEsn());
        }
        return flrStorage;
    }

    private void sendScanMessage(ScanRequest scanRequest) {
        notifyManager.send(TopicConstants.SCAN_REQUEST, JSONObject.fromObject(scanRequest).toString());
        log.info("Sent topic message[{}] successfully", TopicConstants.SCAN_REQUEST);
    }

    /**
     * 生成快照元数据信息
     *
     * @param properties 配置信息
     * @return 元数据信息
     */
    protected abstract String buildSnapMetadata(JSONObject properties);
}
