package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.network.Ipv6AddressUtil;

import lombok.extern.slf4j.Slf4j;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.Locale;

/**
 * 根据集群的IP/port生成对应URI
 *
 * @author cWX1161886
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-17
 */
@Slf4j
public class UriUtil {
    /**
     * Get the storage device uri
     *
     * @param ip ip
     * @param port port
     * @return Storage device uri
     */
    public static URI getUri(String ip, int port) {
        try {
            String linkIp = Ipv6AddressUtil.isIpv6Address(ip) ? "[" + ip + "]" : ip;
            return new URI(String.format(Locale.ENGLISH, "https://%s:%s", linkIp, port));
        } catch (URISyntaxException e) {
            log.error("Get uri failed!", e);
        }
        throw new LegoCheckedException(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT, "Get uri failed");
    }
}

