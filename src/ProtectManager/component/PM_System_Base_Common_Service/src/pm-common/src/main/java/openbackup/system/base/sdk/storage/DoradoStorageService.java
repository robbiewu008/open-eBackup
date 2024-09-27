package openbackup.system.base.sdk.storage;

import openbackup.system.base.common.model.repository.StoragePool;
import openbackup.system.base.common.model.storage.DoradoModifyPwdRequest;
import openbackup.system.base.common.model.storage.StoragePoolParm;
import openbackup.system.base.common.model.storage.StorageResponse;
import openbackup.system.base.common.model.storage.StorageSession;
import openbackup.system.base.common.model.storage.StorageSessionRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import feign.Headers;
import feign.Param;
import feign.RequestLine;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.ResponseBody;

import java.net.URI;
import java.util.List;
import java.util.Map;

/**
 * 功能描述
 *
 * @author y00413474
 * @author w00493811
 * @version [BCManager 8.0.0]
 * @since 2020-06-19
 */
@FeignClient(name = "DoradoStorageService", url = "https://${repository.storage.ip}:${repository.storage.port}",
    configuration = DoradoFeignConfiguration.class)
public interface DoradoStorageService {
    /**
     * 获取存储连接session
     *
     * @param uri uri
     * @param sn sn
     * @param storageSessionRequest 认证信息
     * @return session
     */
    @ExterAttack
    @RequestLine("POST deviceManager/rest/{sn}/sessions")
    @ResponseBody
    StorageResponse<StorageSession> getSession(URI uri, @Param("sn") String sn,
        @RequestBody StorageSessionRequest storageSessionRequest);

    /**
     * 获取存储连接session
     *
     * @param sn sn
     * @param baseToken 认证token
     * @param cookie 认证cookie
     * @return void
     */
    @ExterAttack
    @RequestLine("DELETE deviceManager/rest/{sn}/sessions")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    @ResponseBody
    StorageResponse<Map> deleteSession(@Param("sn") String sn, @Param("iBaseToken") String baseToken,
        @Param("Cookie") String cookie);

    /**
     * 查询存储池
     *
     * @param sn sn
     * @param baseToken 认证token
     * @param cookie 认证cookie
     * @return 存储池
     */
    @ExterAttack
    @RequestLine("GET deviceManager/rest/{sn}/storagepool")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    @ResponseBody
    StorageResponse<List<StoragePool>> getStoragePools(@Param("sn") String sn, @Param("iBaseToken") String baseToken,
        @Param("Cookie") String cookie);

    /**
     * 修改用户密码
     *
     * @param deviceid deviceid
     * @param token 认证token
     * @param cookie 认证cookie
     * @param pwdRequest 修改请求参数
     * @param userName 用户ID
     * @return 存储池
     */
    @ExterAttack
    @RequestLine("PUT deviceManager/rest/{deviceId}/user/{id}")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    @ResponseBody
    StorageResponse<Object> updatePwd(@Param("iBaseToken") String token, @Param("deviceId") String deviceid,
        @Param("Cookie") String cookie, @RequestBody DoradoModifyPwdRequest pwdRequest, @Param("id") String userName);

    /**
     * 修改存储池参数
     *
     * @param token 令牌
     * @param cookie 会话
     * @param deviceId 设备编号
     * @param id 存储池编号
     * @param storagePoolParm 存储池参数
     * @return 修改结果
     */
    @ExterAttack
    @RequestLine("PUT deviceManager/rest/{deviceId}/storagepool/{id}")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    @ResponseBody
    StorageResponse<Object> setStoragePoolAlarmThreshold(@Param("iBaseToken") String token,
        @Param("Cookie") String cookie, @Param("deviceId") String deviceId, @Param("id") String id,
        @RequestBody StoragePoolParm storagePoolParm);
}
