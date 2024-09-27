package openbackup.data.access.client.sdk.api.framework.dme;

import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCancelTask;
import openbackup.data.protection.access.provider.sdk.util.AgentApiUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.service.AvailableAgentManagementDomainService;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.net.URI;

/**
 * 功能描述: DmeUnifiedService
 *
 * @author l00570077
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-06-26
 */
@Slf4j
@Service
@AllArgsConstructor
public class DmeUnifiedService {
    private final DmeUnifiedRestApi dmeUnifiedRestApi;
    private final AvailableAgentManagementDomainService domainService;

    /**
     * 下发即时挂载销毁任务
     *
     * @param task 任务参数
     */
    public void cancelLiveMount(LiveMountCancelTask task) {
        URI uri = domainService.getUrlByAgents(AgentApiUtil.getAgentIds(task.getAgents()));
        log.info("Unmount uri: {}, task id: {}", uri, task.getRequestId());
        if (VerifyUtil.isEmpty(uri)) {
            dmeUnifiedRestApi.cancelLiveMount(task);
        } else {
            dmeUnifiedRestApi.cancelLiveMountWithUri(uri, task);
        }
        log.info("Call dme unmount rest api success, uri: {}, jobId: {}.", uri, task.getRequestId());
    }
}