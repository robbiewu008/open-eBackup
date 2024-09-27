package openbackup.data.access.framework.copy.index.listener.v1;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.index.IndexerProvider;
import openbackup.system.base.common.utils.ExceptionUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Component;

/**
 * 异步新线程调用
 *
 * @author wx1010134
 * @since 2021-05-17
 */
@Slf4j
@Component
public class AsyncGenIndex {
    @Autowired
    private ProviderManager providerManager;

    /**
     * generate index by async
     *
     * @param requestId request id
     * @param copy copy
     */
    @Async
    public void generateIndexFileAsync(String requestId, CopyBo copy) {
        IndexerProvider provider = providerManager.findProvider(IndexerProvider.class, copy.getResourceSubType());
        try {
            provider.generateIndexFile(requestId, copy);
        } catch (Exception e) {
            // 异步线程执行，捕获所有异常
            log.error("Fail to create index, request ID: {}, copy id: {}.", requestId,
                copy.getUuid(), ExceptionUtil.getErrorMessage(e));
        }
    }
}
