package openbackup.system.base.sdk.anti.api;

import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.anti.model.AirGapDeviceInfoRsp;
import openbackup.system.base.sdk.anti.model.AirGapDeviceShowPageReq;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * 功能描述 airGap
 *
 * @author y30037959
 * @since 2023-03-30
 */
@FeignClient(name = "airGapApiSdk", url = "${pm-system-base.url}/v1", configuration = CommonFeignConfiguration.class)
public interface AirGapApi {
    /**
     * 获取AirGap设备列表
     *
     * @param airGapDeviceShowPageRequest airGapDeviceShowPageRequest
     * @return PageListResponse<AirGapDeviceInfo>
     */
    @PostMapping("/internal/anti-ransomware/airgap/devices/page")
    PageListResponse<AirGapDeviceInfoRsp>
        showAirGapDevicePage(@RequestBody AirGapDeviceShowPageReq airGapDeviceShowPageRequest);
}
