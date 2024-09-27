package openbackup.data.access.client.sdk.api.framework.dmc;

import openbackup.data.access.client.sdk.api.config.achive.DmeArchiveFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.context.annotation.Primary;

/**
 * Dmc Copy Service
 *
 * @author l00272247
 * @since 2022-01-17
 */
@FeignClient(
        name = "dmcCopyService",
        url = "${services.endpoints.protectengine.dme}",
        configuration = DmeArchiveFeignConfiguration.class)
@Primary
public interface DmcCopyService extends DmcCopyServiceApi {
}
