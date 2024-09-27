package openbackup.data.access.client.sdk.api.framework.dme.replicate;

import openbackup.data.access.client.sdk.api.config.achive.DmeArchiveFeignConfiguration;
import openbackup.data.access.client.sdk.api.config.achive.DmeResponse;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * DME 高级复制接口
 *
 * @author m00576658
 * @since 2021-01-04
 */
@FeignClient(name = "dmeReplicateService", url = "${services.endpoints.protectengine.replication}",
        configuration = DmeArchiveFeignConfiguration.class)
public interface DmeReplicateService {
    /**
     * 终止复制任务
     *
     * @param advanceReplicationJob advanceReplicationStopJob
     * @return response
     */
    @ExterAttack
    @PostMapping("/v1/dme_replication/task/abort")
    @ResponseBody
    DmeResponse<String> abortReplicationTask(@RequestBody AdvanceReplicationJob advanceReplicationJob);
}
