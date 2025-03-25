/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.common.rest;

import com.fasterxml.jackson.databind.ObjectMapper;

import feign.Client;
import feign.Contract;
import feign.Feign;
import feign.Request;
import feign.RequestInterceptor;
import feign.Retryer;
import feign.Target;
import feign.codec.Decoder;
import feign.codec.Encoder;
import feign.codec.ErrorDecoder;
import feign.form.spring.SpringFormEncoder;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CertCommonConstant;
import openbackup.system.base.common.constants.CertErrorCode;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.FeignClientConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.scurity.BcmX509KeyManager;
import openbackup.system.base.common.scurity.BcmX509TrustManager;
import openbackup.system.base.common.scurity.SecurityCertificateManager;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.config.SystemConfig;

import org.apache.hc.client5.http.ssl.NoopHostnameVerifier;
import org.apache.hc.core5.ssl.SSLContextBuilder;
import org.springframework.beans.factory.ObjectFactory;
import org.springframework.boot.autoconfigure.http.HttpMessageConverters;
import org.springframework.cloud.openfeign.support.SpringEncoder;
import org.springframework.cloud.openfeign.support.SpringMvcContract;
import org.springframework.http.converter.HttpMessageConverter;
import org.springframework.http.converter.json.MappingJackson2HttpMessageConverter;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.Proxy;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.GeneralSecurityException;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;

/**
 * FeignBuilder
 *
 */
@Slf4j
public class FeignBuilder extends Feign.Builder {
    private static final long FIVE_MINUTES = 5 * 60 * 1000L;

    private static long noRetryReadTimeout = 90L;

    private static long noRetryConnectTimeout = 90L;

    private static final String APP_CERT_EXTERNAL_A_8000 = "/app/cert/external/A8000";

    private static final String APP_CERT_EXTERNAL_SYSTEM = "/app/cert/external/EXTERNAL_SYSTEM";

    private static final String APP_CERT_CA = "/app/cert/CA/certs";

    private static final String CA_CRT_PEM = "ca.crt.pem";

    /**
     * 默认连接超时配置：连接超时时间30s，读取超时时间2分钟。
     */
    private static Request.Options defaultTimeoutOptions = new Request.Options(
        FeignClientConstant.CONNECT_TIMEOUT, TimeUnit.MILLISECONDS, FeignClientConstant.READ_TIMEOUT,
        TimeUnit.MILLISECONDS, true);

    private static Request.Options fastFailTimeoutOptions = new Request.Options(
        FeignClientConstant.FAST_FAIL_CONNECT_TIMEOUT, TimeUnit.MILLISECONDS,
        FeignClientConstant.FAST_FAIL_READ_TIMEOUT,
        TimeUnit.MILLISECONDS, true);

    /**
     * VMware连接超时配置：连接超时时间30s，读取超时时间60分钟。
     */
    private static final Request.Options VMWARE_TIMEOUT_OPTIONS = new Request.Options(
        FeignClientConstant.CONNECT_TIMEOUT, TimeUnit.MILLISECONDS, FeignClientConstant.VMWARE_READ_TIMEOUT,
        TimeUnit.MILLISECONDS, true);

    /**
     * 默认连接超时配置：连接超时时间30s，读取超时时间5分钟。
     */
    private static final Request.Options MEMBER_DEFAULT_TIMEOUT_OPTIONS =
        new Request.Options(FeignClientConstant.CONNECT_TIMEOUT, TimeUnit.MILLISECONDS,
            FeignClientConstant.MEMBER_READ_TIMEOUT, TimeUnit.MILLISECONDS, true);

    /**
     * 默认连接超时配置：连接超时时间30s，读取超时时间10秒。
     */
    private static final Request.Options BACKUP_CLUSTER_JOB_DEFAULT_TIMEOUT_OPTIONS =
        new Request.Options(FeignClientConstant.CONNECT_TIMEOUT, TimeUnit.MILLISECONDS,
            FeignClientConstant.BACKUP_CLUSTER_JOB_CLIENT_READ_TIMEOUT, TimeUnit.MILLISECONDS, true);

    /**
     * 路由服务连接超时配置；连接超时时间5s, 读取超时时间10s
     */
    private static final Request.Options ROUTE_SERVICE_TIMEOUT_OPTIONS = new Request.Options(
        FeignClientConstant.ROUTE_SERVICE_CONNECT_TIMEOUT, TimeUnit.MILLISECONDS,
        FeignClientConstant.ROUTE_SERVICE_READ_TIMEOUT, TimeUnit.MILLISECONDS, true);

    /**
     * Dme服务连接超时配置；连接超时时间30s, 读取超时时间5分钟
     */
    private static final Request.Options DME_SERVICE_TIMEOUT_OPTIONS = new Request.Options(
        FeignClientConstant.CONNECT_TIMEOUT, TimeUnit.MILLISECONDS,
        FeignClientConstant.DME_READ_TIMEOUT, TimeUnit.MILLISECONDS, true);

    /**
     * 默认重试策略配置： 当前是重试周期1分钟，最多重试3次。
     */
    private static final Retryer.Default DEFAULT_RETRY_POLICY = new Retryer.Default(FeignClientConstant.PERIOD,
        FeignClientConstant.MAX_PERIOD, FeignClientConstant.MAX_ATTEMPTS);

    static {
        try {
            noRetryReadTimeout = Integer.parseInt(System.getenv("NO_RETRY_READ_TIMEOUT"));
            noRetryConnectTimeout = Integer.parseInt(System.getenv("NO_RETRY_CONNECT_TIMEOUT"));
        } catch (NumberFormatException e) {
            log.error("Unable to read env,use default instead");
        }
    }

    private final Map<String, Object> caches = new HashMap<>();

    public static void setNoRetryConnectTimeout(long noRetryConnectTimeout) {
        FeignBuilder.noRetryConnectTimeout = noRetryConnectTimeout;
    }

    public static void setNoRetryReadTimeout(long noRetryReadTimeout) {
        FeignBuilder.noRetryReadTimeout = noRetryReadTimeout;
    }

    /**
     * 设置默认时间
     *
     * @param connectTimeout connectTimeout
     * @param readTimeout readTimeout
     */
    public static void setDefaultTimeoutOptions(long connectTimeout, long readTimeout) {
        defaultTimeoutOptions = new Request.Options(connectTimeout, TimeUnit.SECONDS, readTimeout,
            TimeUnit.SECONDS, true);
    }

    /**
     * 生成默认FeignBuilder
     *
     * @return 默认FeignBuilder
     */
    public static FeignBuilder getDefaultFeignBuilder() {
        return new FeignBuilder().options(defaultTimeoutOptions);
    }

    /**
     * 生成fast fail FeignBuilder
     *
     * @return fast fail FeignBuilder
     */
    public static Feign.Builder getFastFailFeignBuilder() {
        return new FeignBuilder().options(fastFailTimeoutOptions);
    }

    /**
     * 生成VMwareFeignBuilder
     *
     * @return VMwareFeignBuilder
     */
    public static FeignBuilder getVmwareFeignBuilder() {
        return new FeignBuilder().options(VMWARE_TIMEOUT_OPTIONS);
    }

    /**
     * 生成路由服务FeignBuilder
     *
     * @return RouteServiceFeignBuilder
     */
    public static FeignBuilder getRouteServiceFeignBuilder() {
        return new FeignBuilder().options(ROUTE_SERVICE_TIMEOUT_OPTIONS);
    }

    /**
     * 生成Dme服务FeignBuilder
     *
     * @return DmeServiceFeignBuilder
     */
    public static FeignBuilder getDmeServiceFeignBuilder() {
        return new FeignBuilder().options(DME_SERVICE_TIMEOUT_OPTIONS);
    }

    /**
     * 生成默认Builder，包含默认的重试策略
     *
     * @return 默认Builder
     */
    public static Feign.Builder getDefaultRetryableBuilder() {
        return getDefaultFeignBuilder().retryer(DEFAULT_RETRY_POLICY);
    }

    /**
     * build target with default config
     *
     * @param type type
     * @param encoder encoder
     * @param <T> template type
     * @return target
     */
    public static <T> T buildConfigWithDefaultConfig(Class<T> type, Encoder encoder) {
        return getDefaultRetryableBuilder().encoder(getEncoder(encoder))
            .decoder(CommonDecoder.decoder())
            .client(getClient())
            .errorDecoder(CommonDecoder::errorDecode)
            .contract(new feign.Contract.Default())
            .target(Target.EmptyTarget.create(type));
    }

    private static Encoder getEncoder(Encoder encoder) {
        return Optional.ofNullable(encoder).orElseGet(FeignBuilder::createDefaultEncoder);
    }

    private static Encoder createDefaultEncoder() {
        ObjectFactory<HttpMessageConverters> httpMessageConverters = getHttpMessageConvertersObjectFactory();
        return new SpringEncoder(httpMessageConverters);
    }

    /**
     * build target with default config
     *
     * @param type type
     * @param encoder encoder
     * @param <T> template type
     * @param isVerifyCert verify cert or not
     * @return target
     */
    public static <T> T buildHttpsTarget(Class<T> type, Encoder encoder, boolean isVerifyCert) {
        return buildHttpsTarget(type, encoder, isVerifyCert, false, null);
    }

    /**
     * build target with proxy client
     *
     * @param <T> 模板类型
     * @param type FeignClient类型
     * @param encoder 编码器
     * @param isVerifyCert 是否需要验证证书
     * @param isTransferCert 是否需要传送证书
     * @param proxy dme代理
     * @return 创建好的FeignClient
     */
    public static <T> T buildHttpsTarget(Class<T> type, Encoder encoder, boolean isVerifyCert, boolean isTransferCert,
        Proxy proxy) {
        return getFeignBuilder(encoder, isVerifyCert, isTransferCert, proxy).target(Target.EmptyTarget.create(type));
    }

    /**
     * build target with proxy client
     *
     * @param <T> 模板类型
     * @param type FeignClient类型
     * @param encoder 编码器
     * @param isVerifyCert 是否需要验证证书
     * @param isTransferCert 是否需要传送证书
     * @param proxy dme代理
     * @return 创建好的FeignClient
     */
    public static <T> T buildLongTimeRetryApi(Class<T> type, Encoder encoder, boolean isVerifyCert,
            boolean isTransferCert, Proxy proxy) {
        Request.Options options = new Request.Options(FeignClientConstant.CONNECT_TIMEOUT, TimeUnit.MILLISECONDS,
                FIVE_MINUTES, TimeUnit.MILLISECONDS, true);
        Feign.Builder builder = new FeignBuilder()
                .options(options)
                .retryer(DEFAULT_RETRY_POLICY)
                .encoder(getEncoder(encoder))
                .decoder(CommonDecoder.decoder())
                .errorDecoder(CommonDecoder::errorDecode)
                .contract(new Contract.Default());
        if (isVerifyCert) {
            builder.client(buildTrustManagerClient(isTransferCert, proxy, null));
        } else {
            builder.client(getClient());
        }
        return builder.target(Target.EmptyTarget.create(type));
    }

    /**
     * build target with proxy client
     *
     * @param <T> 模板类型
     * @param type FeignClient类型
     * @param encoder 编码器
     * @param isVerifyCert 是否需要验证证书
     * @param isTransferCert 是否需要传送证书
     * @param proxy dme代理
     * @return 创建好的FeignClient
     */
    public static <T> T buildNoRetryApi(Class<T> type, Encoder encoder, boolean isVerifyCert,
                                        boolean isTransferCert, Proxy proxy) {
        Request.Options options = new Request.Options(noRetryConnectTimeout, TimeUnit.SECONDS,
                noRetryReadTimeout, TimeUnit.SECONDS, true);
        Feign.Builder builder = new FeignBuilder()
                .options(options)
                .retryer(Retryer.NEVER_RETRY)
                .encoder(getEncoder(encoder))
                .decoder(CommonDecoder.decoder())
                .errorDecoder(CommonDecoder::errorDecode)
                .contract(new Contract.Default());
        if (isVerifyCert) {
            builder.client(buildTrustManagerClient(isTransferCert, proxy, null));
        } else {
            builder.client(getClient());
        }
        return builder.target(Target.EmptyTarget.create(type));
    }

    private static Feign.Builder getFeignBuilder(Encoder encoder, boolean isVerifyCert, boolean isTransferCert,
        Proxy proxy) {
        Feign.Builder builder = getDefaultRetryableBuilder().encoder(getEncoder(encoder))
            .decoder(CommonDecoder.decoder())
            .errorDecoder(CommonDecoder::errorDecode)
            .contract(new Contract.Default());
        if (isVerifyCert) {
            builder.client(buildTrustManagerClient(isTransferCert, proxy, null));
        } else {
            builder.client(getClient());
        }
        return builder;
    }

    /**
     * build target with default config for internal request
     *
     * @param type type
     * @param encoder encoder
     * @param client client
     * @param <T> template type
     * @return target
     */
    public static <T> T buildInternalHttpsTarget(Class<T> type, Encoder encoder, Client client) {
        return getDefaultRetryableBuilder().client(client)
            .encoder(getEncoder(encoder))
            .decoder(CommonFeignConfiguration.decoder())
            .contract(new feign.Contract.Default())
            .target(Target.EmptyTarget.create(type));
    }

    /**
     * build target with default config
     *
     * @param type type
     * @param encoder encoder
     * @param <T> template type
     * @param isVerifyCert verify cert or not
     * @param proxy proxy
     * @return target
     */
    public static <T> T buildHttpsTargetWithProxy(Class<T> type, Encoder encoder, boolean isVerifyCert, Proxy proxy) {
        return buildHttpsTarget(type, encoder, isVerifyCert, false, proxy);
    }

    /**
     * 构造访问外部集群的客户端，使用Feign默认的超时重试机制，避免超时时间过长
     *
     * @param type type
     * @param encoder encoder
     * @param proxy proxy
     * @param <T> template type
     * @return target
     */
    public static <T> T buildTargetClusterClient(Class<T> type, Encoder encoder, Proxy proxy) {
        return new FeignBuilder()
                .retryer(Retryer.NEVER_RETRY)
                .encoder(getEncoder(encoder))
                .decoder(CommonDecoder.decoder())
                .errorDecoder(CommonDecoder::errorDecode)
                .contract(new Contract.Default())
                .client(buildTrustManagerClient(false, proxy, null))
                .target(Target.EmptyTarget.create(type));
    }

    /**
     * 构造访问外部复制集群的客户端，使用Feign默认的超时重试机制，避免超时时间过长
     *
     * @param type type
     * @param encoder encoder
     * @param proxy proxy
     * @param <T> template type
     * @return target
     */
    public static <T> T buildDefaultTargetClusterClient(Class<T> type, Encoder encoder, Proxy proxy) {
        return new FeignBuilder()
            .retryer(Retryer.NEVER_RETRY)
            .encoder(getEncoder(encoder))
            .decoder(CommonDecoder.decoder())
            .errorDecoder(CommonDecoder::errorDecode)
            .contract(new Contract.Default())
            .client(buildTrustManagerClient(false, proxy, getKeyStore()))
            .target(Target.EmptyTarget.create(type));
    }

    /**
     * 构造访问备份成员节点的客户端，设置超时时间为5分钟
     *
     * @param type type
     * @param encoder encoder
     * @param proxy proxy
     * @param <T> template type
     * @return target
     */
    public static <T> T buildMemberClusterClient(Class<T> type, Encoder encoder, Proxy proxy) {
        return new FeignBuilder().options(MEMBER_DEFAULT_TIMEOUT_OPTIONS)
            .retryer(Retryer.NEVER_RETRY)
            .encoder(getEncoder(encoder))
            .decoder(CommonDecoder.decoder())
            .errorDecoder(CommonDecoder::errorDecode)
            .contract(new Contract.Default())
            .client(buildTrustManagerClient(false, proxy, getKeyStore()))
            .target(Target.EmptyTarget.create(type));
    }

    /**
     * 构造访问备份成员节点的客户端，设置超时时间为10秒
     *
     * @param type type
     * @param encoder encoder
     * @param proxy proxy
     * @param <T> template type
     * @return target
     */
    public static <T> T buildBackupClusterJobClient(Class<T> type, Encoder encoder, Proxy proxy) {
        return new FeignBuilder().options(BACKUP_CLUSTER_JOB_DEFAULT_TIMEOUT_OPTIONS)
            .retryer(Retryer.NEVER_RETRY)
            .encoder(getEncoder(encoder))
            .decoder(CommonDecoder.decoder())
            .errorDecoder(CommonDecoder::errorDecode)
            .contract(new Contract.Default())
            .client(buildTrustManagerClient(false, proxy, getKeyStore()))
            .target(Target.EmptyTarget.create(type));
    }

    /**
     * 构造访问备份成员节点的客户端，支持传输文件
     *
     * @param type type
     * @param proxy proxy
     * @param <T> template type
     * @return memberClusterApi
     */
    public static <T> T buildDefaultMemberClusterClient(Class<T> type, Proxy proxy) {
        return new FeignBuilder().options(MEMBER_DEFAULT_TIMEOUT_OPTIONS)
            .retryer(Retryer.NEVER_RETRY)
            .encoder(new SpringFormEncoder())
            .decoder(CommonDecoder.decoder())
            .errorDecoder(CommonDecoder::errorDecode)
            .client(buildTrustManagerClient(false, proxy, getKeyStore()))
            .contract(new SpringMvcContract())
            .target(Target.EmptyTarget.create(type));
    }

    /**
     * 获取hcs feign
     *
     * @param type api类型
     * @param isVerifyCert 是否校验证书
     * @param isVerifyHostName 是否校验域名
     * @param proxy 代理
     * @param <T> 类型
     *
     * @return feign
     */
    public static <T> T buildHcsApiClient(Class<T> type, boolean isVerifyCert, boolean isVerifyHostName, Proxy proxy) {
        Feign.Builder builder = new FeignBuilder().options(defaultTimeoutOptions)
            .retryer(DEFAULT_RETRY_POLICY)
            .encoder(getEncoder(null))
            .decoder(CommonDecoder.decoder())
            .errorDecoder(CommonDecoder::errorDecode)
            .contract(new Contract.Default());
        if (isVerifyCert) {
            builder.client(buildTrustManagerClientByParam(false, proxy, null, isVerifyHostName));
        }
        if (proxy == null) {
            builder.client(getClient());
        } else {
            builder.client(getProxyClient(proxy));
        }
        return builder.target(Target.EmptyTarget.create(type));
    }

    /**
     * 构造访问外部系统的客户端，使用Feign默认的超时重试机制，避免超时时间过长
     *
     * @param type type
     * @param encoder encoder
     * @param proxy proxy
     * @param <T> template type
     * @return target
     */
    public static <T> T buildExternalSystemClient(Class<T> type, Encoder encoder, Proxy proxy) {
        return new FeignBuilder().retryer(Retryer.NEVER_RETRY)
            .encoder(getEncoder(encoder))
            .decoder(CommonDecoder.decoder())
            .errorDecoder(CommonDecoder::errorDecode)
            .contract(new Contract.Default())
            .client(buildTrustManagerClient(false, proxy, getKeyStore(APP_CERT_EXTERNAL_SYSTEM)))
            .target(Target.EmptyTarget.create(type));
    }

    private static KeyStore getKeyStore(String path) {
        File[] certFiles = Paths.get(path).toFile().listFiles();
        if (VerifyUtil.isEmpty(certFiles) || Objects.isNull(certFiles[0]) || Files.notExists(certFiles[0].toPath())) {
            throw new LegoCheckedException(CertErrorCode.CA_CERTIFICATE_IS_INVALID, "invalid cert");
        }
        List<File> certFileList = new ArrayList<>(Arrays.asList(certFiles));
        Optional<File> caCertPemFile = getCaCertPemFile();
        caCertPemFile.ifPresent(certFileList::add);
        log.debug("Start to load File:{}", certFiles.length);
        KeyStore ks;
        try {
            ks = KeyStore.getInstance(KeyStore.getDefaultType());
            ks.load(null);
        } catch (KeyStoreException | IOException | NoSuchAlgorithmException | CertificateException e) {
            log.error("create Cert Trust Mananger error", e);
            throw new LegoCheckedException(CertErrorCode.CA_CERTIFICATE_IS_INVALID, "invalid cert");
        }
        for (File certFile : certFileList) {
            try (FileInputStream fis = new FileInputStream(certFile)) {
                CertificateFactory cf = CertificateFactory.getInstance("X509");
                Certificate cert = cf.generateCertificate(fis);
                X509Certificate x509Cert;
                if (cert instanceof X509Certificate) {
                    x509Cert = (X509Certificate) cert;
                } else {
                    throw new LegoCheckedException(CertErrorCode.CA_CERTIFICATE_IS_INVALID, "invalid cert");
                }
                log.debug("Start to load File:{}", certFile.getName());
                ks.setCertificateEntry(certFile.getName(), x509Cert);
            } catch (IOException | GeneralSecurityException e) {
                log.error("create Cert Trust Mananger error", e);
            }
        }
        return ks;
    }

    private static KeyStore getKeyStore() {
        return getKeyStore(APP_CERT_EXTERNAL_A_8000);
    }

    private static Optional<File> getCaCertPemFile() {
        File caCertFile = Paths.get(APP_CERT_CA, CA_CRT_PEM).toFile();
        if (Files.notExists(caCertFile.toPath())) {
            return Optional.empty();
        }
        return Optional.of(caCertFile);
    }

    /**
     * Build dorado feign rest client
     *
     * @param type API CLASS TYPE
     * @param decoder response decoder
     * @param errorDecoder response error decoder
     * @param proxy proxy
     * @return the instance of API class
     */
    public static <T> T buildDoradoHttpsClient(Class<T> type, Decoder decoder, ErrorDecoder errorDecoder, Proxy proxy) {
        Feign.Builder builder = getDefaultRetryableBuilder()
            .encoder(createDefaultEncoder())
            .decoder(decoder)
            .errorDecoder(errorDecoder)
            .contract(new Contract.Default());
        builder.client(buildTrustManagerClient(false, proxy, null));
        return builder.target(Target.EmptyTarget.create(type));
    }

    /**
     * Build feign rest client
     *
     * @param type API CLASS TYPE
     * @param decoder response decoder
     * @param errorDecoder response error decoder
     * @param proxy proxy
     * @param keyStore key store
     * @return the instance of API class
     */
    public static <T> T buildHttpsClient(Class<T> type, Decoder decoder, ErrorDecoder errorDecoder, Proxy proxy,
        KeyStore keyStore) {
        return getDefaultRetryableBuilder().encoder(createDefaultEncoder())
            .decoder(decoder)
            .errorDecoder(errorDecoder)
            .contract(new Contract.Default())
            .client(buildTrustManagerClient(false, proxy, keyStore))
            .target(Target.EmptyTarget.create(type));
    }

    /**
     * Build feign rest client no retry
     *
     * @param type API CLASS TYPE
     * @param decoder response decoder
     * @param errorDecoder response error decoder
     * @param proxy proxy
     * @param keyStore key store
     * @return the instance of API class
     */
    public static <T> T buildHttpsClientNoRetry(Class<T> type, Decoder decoder, ErrorDecoder errorDecoder, Proxy proxy,
                                                KeyStore keyStore) {
        Request.Options options = new Request.Options(noRetryConnectTimeout, TimeUnit.SECONDS,
                noRetryReadTimeout, TimeUnit.SECONDS, true);
        return new FeignBuilder().options(options)
            .encoder(createDefaultEncoder())
            .decoder(decoder)
            .errorDecoder(errorDecoder)
            .contract(new Contract.Default())
            .client(buildTrustManagerClient(false, proxy, keyStore))
            .target(Target.EmptyTarget.create(type));
    }

    /**
     * 创建内部feignClient
     *
     * @param type type
     * @param client client
     * @param <T> template type
     * @return target
     */
    public static <T> T buildInternalHttpsClient(Class<T> type, Client client) {
        return getDefaultRetryableBuilder().encoder(createDefaultEncoder())
            .decoder(CommonDecoder.decoder())
            .errorDecoder(CommonDecoder::errorDecode)
            .contract(new Contract.Default())
            .client(client)
            .target(Target.EmptyTarget.create(type));
    }

    /**
     * 创建内部feignClient
     *
     * @param type type
     * @param client client
     * @param <T> template type
     * @return target
     */
    public static <T> T buildInternalHttpsClientWithSpringMvcContractDefaultEncoder(Class<T> type, Client client) {
        return getDefaultFeignBuilder().encoder(createDefaultEncoder())
            .decoder(CommonDecoder.decoder())
            .errorDecoder(CommonDecoder::errorDecode)
            .contract(new SpringMvcContract())
            .client(client)
            .target(Target.EmptyTarget.create(type));
    }

    /**
     * 创建内部feignClient
     *
     * @param type type
     * @param client client
     * @param <T> template type
     * @return target
     */
    public static <T> T buildInternalHttpsClientWithSpringMvcContract(Class<T> type, Client client) {
        return getDefaultFeignBuilder().encoder(new SpringFormEncoder())
            .decoder(CommonDecoder.decoder())
            .errorDecoder(CommonDecoder::errorDecode)
            .contract(new SpringMvcContract())
            .client(client)
            .target(Target.EmptyTarget.create(type));
    }

    /**
     * Build feign rest client
     *
     * @param type API CLASS TYPE
     * @param decoder response decoder
     * @param errorDecoder response error decoder
     * @param url target url
     * @param interceptor Request interceptor
     * @param <T> API class
     * @return the instance of API class
     */
    public static <T> T build(Class<T> type, Decoder decoder, ErrorDecoder errorDecoder, String url,
        RequestInterceptor interceptor) {
        ObjectFactory<HttpMessageConverters> converter = getHttpMessageConvertersObjectFactory();
        Feign.Builder builder = getDefaultRetryableBuilder().encoder(new SpringEncoder(converter))
            .decoder(decoder)
            .client(getClient())
            .errorDecoder(errorDecoder)
            .contract(new feign.Contract.Default());
        if (interceptor != null) {
            builder.requestInterceptor(interceptor);
        }
        if (url != null) {
            return builder.target(type, url);
        } else {
            return builder.target(Target.EmptyTarget.create(type));
        }
    }

    /**
     * Build feign rest client with retryer
     *
     * @param type API CLASS TYPE
     * @param retryer Request retryer
     * @param decoder response decoder
     * @param errorDecoder response error decoder
     * @param url target url
     * @param <T> API class
     * @return the instance of API class
     */
    public static <T> T build(Class<T> type, Retryer retryer, Decoder decoder, ErrorDecoder errorDecoder, String url) {
        ObjectFactory<HttpMessageConverters> converter = getHttpMessageConvertersObjectFactory();
        Feign.Builder builder = getDefaultFeignBuilder().encoder(new SpringEncoder(converter))
            .decoder(decoder)
            .client(getClient())
            .errorDecoder(errorDecoder)
            .contract(new feign.Contract.Default())
            .retryer(retryer);
        if (url != null) {
            return builder.target(type, url);
        } else {
            return builder.target(Target.EmptyTarget.create(type));
        }
    }

    /**
     * Build feign rest client with proxy
     *
     * @param type API CLASS TYPE
     * @param decoder response decoder
     * @param errorDecoder response error decoder
     * @param proxy proxy
     * @param <T> API class
     * @return the instance of API class
     */
    public static <T> T buildProxy(Class<T> type, Decoder decoder, ErrorDecoder errorDecoder, Proxy proxy) {
        ObjectFactory<HttpMessageConverters> converter = getHttpMessageConvertersObjectFactory();
        Feign.Builder builder = getDefaultRetryableBuilder().encoder(new SpringEncoder(converter))
            .decoder(decoder)
            .client(getProxyClient(proxy))
            .errorDecoder(errorDecoder)
            .contract(new feign.Contract.Default());
        return builder.target(Target.EmptyTarget.create(type));
    }

    /**
     * Build feign rest client with proxy and no retry
     *
     * @param type API CLASS TYPE
     * @param decoder response decoder
     * @param errorDecoder response error decoder
     * @param proxy proxy
     * @param <T> API class
     * @return the instance of API class
     */
    public static <T> T buildProxyWithoutRetry(Class<T> type, Decoder decoder, ErrorDecoder errorDecoder, Proxy proxy) {
        ObjectFactory<HttpMessageConverters> converter = getHttpMessageConvertersObjectFactory();
        Feign.Builder builder = getFastFailFeignBuilder().retryer(Retryer.NEVER_RETRY)
            .encoder(new SpringEncoder(converter))
            .decoder(decoder)
            .client(getProxyClient(proxy))
            .errorDecoder(errorDecoder)
            .contract(new feign.Contract.Default());
        return builder.target(Target.EmptyTarget.create(type));
    }

    /**
     * Build feign rest client with proxy
     *
     * @param type API CLASS TYPE
     * @param proxy proxy
     * @param <T> API class
     * @return the instance of API class
     */
    public static <T> T buildExternalDoradoApiWithProxy(Class<T> type, Proxy proxy) {
        return getDefaultRetryableBuilder()
            .encoder(createDefaultEncoder())
            .decoder(CommonDecoder.decoder())
            .errorDecoder(CommonDecoder::errorDecode)
            .contract(new Contract.Default())
            .client(getProxyClient(proxy))
            .target(Target.EmptyTarget.create(type));
    }

    private static ObjectFactory<HttpMessageConverters> getHttpMessageConvertersObjectFactory() {
        HttpMessageConverter<?> jsonConverter = new MappingJackson2HttpMessageConverter(new ObjectMapper());
        return () -> new HttpMessageConverters(jsonConverter);
    }

    /**
     * 获取Client
     *
     * @return Client
     */
    private static Client getClient() {
        Client client;
        try {
            client = ApacheHttp5ClientBuilder.buildSslNoVerifyClient();
        } catch (NoSuchAlgorithmException | KeyStoreException | KeyManagementException e) {
            throw LegoCheckedException.cast(e, CommonErrorCode.OPERATION_FAILED,
                "Create feign client with SSL config failed");
        }
        return client;
    }

    private static Client getProxyClient(Proxy proxy) {
        Client client;
        try {
            SSLContext context = new SSLContextBuilder().loadTrustMaterial(null, (chain, authType) -> true).build();
            client = new Client.Proxied(context.getSocketFactory(), new NoopHostnameVerifier(), proxy);
        } catch (NoSuchAlgorithmException | KeyStoreException | KeyManagementException e) {
            throw LegoCheckedException.cast(e, CommonErrorCode.OPERATION_FAILED,
                "Create feign proxy client with SSL config failed");
        }
        return client;
    }

    /**
     * 获取服务器SSL验证的client
     *
     * @param isTransferCert 是否需要传送证书
     * @param proxy dme代理
     * @param keyStore 证书库
     * @return client
     */
    private static Client buildTrustManagerClient(boolean isTransferCert, Proxy proxy, KeyStore keyStore) {
        return getClient(isTransferCert, proxy, keyStore, false);
    }

    private static Client buildTrustManagerClientByParam(boolean isTransferCert, Proxy proxy, KeyStore keyStore,
        boolean isVerifyHostName) {
        return getClient(isTransferCert, proxy, keyStore, isVerifyHostName);
    }

    private static Client getClient(boolean isTransferCert, Proxy proxy, KeyStore keyStore, boolean isVerifyHostName) {
        Client client;
        try {
            SSLContext ctx = SSLContext.getInstance("TLSv1.2");
            KeyManager[] keyManagers = getKeyManagers(isTransferCert);

            if (keyStore == null) {
                ctx.init(new KeyManager[] {new BcmX509KeyManager(
                                CertCommonConstant.PM_SERVER_CERT_IN_KEYSTORE_NAME,
                                SystemConfig.getInstance().getKeyPass().toCharArray())},
                        new TrustManager[] {new BcmX509TrustManager(new CommonX509TrustHandler())},
                    SecureRandom.getInstanceStrong());
            } else {
                ctx.init(keyManagers,
                    new TrustManager[] {new BcmX509TrustManager(keyStore, new CommonX509TrustHandler())},
                    SecureRandom.getInstanceStrong());
            }

            // 根据传参确定是否开启域名校验
            HostnameVerifier hv = isVerifyHostName
                ? HttpsURLConnection.getDefaultHostnameVerifier()
                : (urlHostName, session) -> true;
            if (proxy == null) {
                client = new Client.Default(ctx.getSocketFactory(), hv);
            } else {
                client = new Client.Proxied(ctx.getSocketFactory(), hv, proxy);
            }
        } catch (NoSuchAlgorithmException | KeyManagementException | KeyStoreException | UnrecoverableKeyException e) {
            throw LegoCheckedException.cast(e, CommonErrorCode.OPERATION_FAILED,
                "Create trust feign client with SSL config failed");
        }
        return client;
    }

    private static KeyManager[] getKeyManagers(boolean isTransferCert)
        throws NoSuchAlgorithmException, KeyStoreException, UnrecoverableKeyException {
        if (!isTransferCert) {
            return new KeyManager[] {};
        }
        KeyStore keyStore = SecurityCertificateManager.getKeyStore();
        KeyManagerFactory km = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
        km.init(keyStore, SystemConfig.getInstance().getKeyPass().toCharArray());
        return km.getKeyManagers();
    }

    /**
     * retryer
     *
     * @param retryer retryer
     * @return feign builder
     */
    @Override
    public Feign.Builder retryer(Retryer retryer) {
        return init("retryer", super::retryer, retryer);
    }

    /**
     * decoder
     *
     * @param decoder decoder
     * @return feign builder
     */
    @Override
    public Feign.Builder decoder(Decoder decoder) {
        return init("decoder", super::decoder, decoder);
    }

    /**
     * encoder
     *
     * @param encoder encoder
     * @return feign builder
     */
    @Override
    public Feign.Builder encoder(Encoder encoder) {
        return init("encoder", super::encoder, encoder);
    }

    /**
     * error decoder
     *
     * @param errorDecoder error decoder
     * @return feign builder
     */
    @Override
    public Feign.Builder errorDecoder(ErrorDecoder errorDecoder) {
        return init("errorDecoder", super::errorDecoder, errorDecoder);
    }

    @Override
    public FeignBuilder options(Request.Options options) {
        super.options(options);
        return this;
    }

    private <T> FeignBuilder init(String name, Consumer<T> consumer, T value) {
        Object cache = caches.get(name);
        if (cache == null || cache.getClass().getName().startsWith("feign.")) {
            consumer.accept(value);
            caches.put(name, value);
        }
        return this;
    }
}
