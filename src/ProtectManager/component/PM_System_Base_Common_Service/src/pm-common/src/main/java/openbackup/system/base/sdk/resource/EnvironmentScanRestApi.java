package openbackup.system.base.sdk.resource;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestParam;

/**
 * 功能描述
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-08-28
 */
@FeignClient(name = "EnvironmentScanRestApi",
    url = "${services.endpoints.protectmanager.protection-service}/v1/internal",
    configuration = CommonFeignConfiguration.class)
public interface EnvironmentScanRestApi {
    /**
     * protection service 手动资源扫描
     *
     * @param envId 环境id
     * @param jobId 任务id
     * @param subtype 资源子类型
     */
    @ExterAttack
    @PutMapping("/environments/rescan/{envId}")
    void doScanResource(@PathVariable("envId") String envId, @RequestParam("job_id") String jobId,
        @RequestParam("subtype") String subtype);
}
