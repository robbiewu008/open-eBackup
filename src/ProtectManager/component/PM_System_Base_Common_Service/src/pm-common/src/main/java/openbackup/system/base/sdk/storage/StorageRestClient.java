package openbackup.system.base.sdk.storage;

import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;

/**
 * Storage Client Rest Api
 *
 * @author l00272247
 * @since 2020-1130
 */
@FeignClient(name = "storage-client", url = "${service.url.pm-system-base}/v1",
    configuration = CommonFeignConfiguration.class)
public interface StorageRestClient extends StorageRestApi {
}