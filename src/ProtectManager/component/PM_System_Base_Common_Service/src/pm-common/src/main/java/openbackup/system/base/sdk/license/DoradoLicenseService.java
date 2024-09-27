package openbackup.system.base.sdk.license;

import openbackup.system.base.sdk.license.module.ActiveLicenseRes;
import openbackup.system.base.sdk.license.module.DoradoLicenseResponse;
import openbackup.system.base.security.exterattack.ExterAttack;

import feign.Headers;
import feign.Param;
import feign.RequestLine;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;
import java.util.Map;

/**
 * 功能描述
 *
 * @author s00455050
 * @since 2021-01-28
 */
@FeignClient(value = "DoradoLicenseService", url = "https://${repository.storage.ip}:${repository.storage.port}",
    configuration = LicenseFeignConfiguration.class)
public interface DoradoLicenseService {
    /**
     * 查询license features和features状态
     *
     * @param deviceId 设备id
     * @param baseToken 需要携带的会话iBaseToken
     * @param cookie cookie
     * @return license列表 格式如[{"HyperSnap":1"id":1},{ "HyperClone":1"id":2}, {"HyperCopy":1"id":3}]
     */
    @ExterAttack
    @RequestLine("GET /deviceManager/rest/{deviceId}/license/feature")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    @ResponseBody
    DoradoLicenseResponse<List<Map>> queryLicense(@Param("deviceId") String deviceId,
        @Param("iBaseToken") String baseToken, @Param("Cookie") String cookie);

    /**
     * 查询license, 包括license容量
     *
     * @param deviceId 设备id
     * @param baseToken 需要携带的会话iBaseToken
     * @param cookie cookie
     * @return license 容量信息
     */
    @ExterAttack
    @RequestLine("GET /deviceManager/rest/{deviceId}/license/activelicense")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    @ResponseBody
    DoradoLicenseResponse<ActiveLicenseRes> getActiveLicense(@Param("deviceId") String deviceId,
        @Param("iBaseToken") String baseToken, @Param("Cookie") String cookie);
}
