package openbackup.system.base.sdk.config.api;

import openbackup.system.base.security.exterattack.ExterAttack;

import feign.RequestLine;

import java.net.URI;
import java.util.List;

/**
 * PmConfigRestApi
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-07-24
 */
public interface PmConfigRestApi {
    /**
     * 获取网络配置
     *
     * @param uri uri
     * @return 网络配置
     */
    @ExterAttack
    @RequestLine("GET /v1/internal/pm-config/network/local-network")
    List<String> getLocalNetwork(URI uri);
}
