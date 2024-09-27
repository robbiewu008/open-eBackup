package openbackup.system.base.util;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.rest.ApacheHttp5ClientBuilder;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.config.DmaProxyProperties;
import openbackup.system.base.config.DmeProxyProperties;

import feign.Client;
import lombok.extern.slf4j.Slf4j;

import org.springframework.web.util.UriComponents;
import org.springframework.web.util.UriComponentsBuilder;

import java.net.InetSocketAddress;
import java.net.Proxy;
import java.net.URI;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;

/**
 * 获取请求url util
 *
 * @author z30009433
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-24
 */
@Slf4j
public class RequestUriUtil {
    // 端口支持的最小值
    private static final Integer MIN_PORT_NUMBER = 1;

    // 端口支持的最大值
    private static final Integer MAX_PORT_NUMBER = 65535;

    // http请求scheme
    private static final String REQUEST_SCHEME = "https";

    private RequestUriUtil() {
    }

    /**
     * 获取请求url
     *
     * @param endpoint 请求ip
     * @param port 请求port
     * @return 请求url
     */
    public static URI getRequestUri(String endpoint, Integer port) {
        verifyIpAndPort(endpoint, port);
        UriComponents uriComponents = UriComponentsBuilder.newInstance()
            .scheme(REQUEST_SCHEME)
            .host(endpoint)
            .port(port)
            .build();
        log.debug("Host environment get request uri.");
        return uriComponents.toUri();
    }

    /**
     * 获取代理对象
     *
     * @param proxyIp 代理ip
     * @param proxyPort 代理port
     * @return Proxy
     */
    public static Proxy getProxy(String proxyIp, Integer proxyPort) {
        return new Proxy(Proxy.Type.HTTP, new InetSocketAddress(proxyIp, proxyPort));
    }

    /**
     * 校验ip和port，ip和port不能为空，且port不能超过一定的范围
     *
     * @param ip 被校验的ip
     * @param port 被校验的port
     */
    public static void verifyIpAndPort(String ip, Integer port) {
        if (VerifyUtil.isEmpty(ip)) {
            throw new LegoCheckedException(CommonErrorCode.IP_PORT_ERROR, "ip can not be empty.");
        }
        if (VerifyUtil.isEmpty(port) || !checkIsLegalPort(port)) {
            throw new LegoCheckedException(CommonErrorCode.IP_PORT_ERROR, "port can not be empty and out of range.");
        }
    }

    /**
     * 获取dma 代理
     *
     * @param proxyProperties dma 域名和端口
     * @return dma代理
     */
    public static Proxy getDmaProxy(DmaProxyProperties proxyProperties) {
        return getProxy(proxyProperties.getHost(), proxyProperties.getPort());
    }

    /**
     * 获取 DME 代理
     *
     * @param proxyProperties DME 域名和端口
     * @return DME 代理
     */
    public static Proxy getDmeProxy(DmeProxyProperties proxyProperties) {
        return getProxy(proxyProperties.getIp(), proxyProperties.getProxyPort());
    }

    /**
     * 获取客户端
     *
     * @return client 客户端
     */
    public static Client getNoVerifyClient() {
        Client client;
        try {
            client = ApacheHttp5ClientBuilder.buildSslNoVerifyClient();
        } catch (KeyStoreException | NoSuchAlgorithmException | KeyManagementException e) {
            client = ApacheHttp5ClientBuilder.buildDefaultClient();
        }
        return client;
    }

    private static boolean checkIsLegalPort(Integer port) {
        return port >= MIN_PORT_NUMBER && port <= MAX_PORT_NUMBER;
    }
}
