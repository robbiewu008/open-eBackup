package openbackup.system.base.common.utils;

import openbackup.system.base.common.errors.ErrorConstant;
import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;
import openbackup.system.base.common.exception.EmeiStorUnauthorizedException;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.scurity.KeyStoreLoader;
import openbackup.system.base.common.scurity.Watcher;
import openbackup.system.base.config.SystemConfig;

import org.bouncycastle.util.encoders.Base64;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.security.Key;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;

/**
 * KeyStore工具类
 *
 * @author y00407642
 * @since 2020-06-15
 */
public class KeystoreUtils {
    private static final Logger LOGGER = LoggerFactory.getLogger(KeystoreUtils.class);

    private static final KeystoreUtils KEYSTORE_UTILS = new KeystoreUtils();

    private static String alias;

    private static String privateKeyPassword;

    private static KeyStoreLoader loader;

    private static PublicKey publicKey;

    private static String certificate;

    private static Certificate cert;

    private static boolean isInitSuccess = false;

    private KeystoreUtils() {
    }

    /**
     * get instance
     *
     * @return instance
     */
    public static KeystoreUtils getInstance() {
        checkInit();
        return KEYSTORE_UTILS;
    }

    private static synchronized void checkInit() {
        if (!isInitSuccess) {
            SystemConfig config = SystemConfig.getInstance();
            alias = config.getKeyAlias();
            String keyStorePath = config.getKeystoreFile();
            String keyStorePass = config.getKeystorePass();
            privateKeyPassword = config.getKeyPass();
            loader = new KeyStoreLoader(
                    "PKCS12", keyStorePath, keyStorePass::toString).handle(KeystoreUtils::handle);
            loader.start();
            isInitSuccess = true;
        }
    }

    /**
     * 获取公钥
     *
     * @return 公钥
     */
    public PublicKey getPublicKey() {
        return publicKey;
    }

    public static Certificate getCert() {
        return cert;
    }

    /**
     * 获取私钥
     *
     * @return 私钥
     */
    public PrivateKey getPrivateKey() {
        LOGGER.info("Start get ks file.");
        KeyStore keyStore;
        try {
            keyStore = loader.getKeyStore();
        } catch (LegoCheckedException ex) {
            LOGGER.error("Get ks file error.", ExceptionUtil.getErrorMessage(ex));
            throw ex;
        }
        LOGGER.info("Get ks file success.");
        try {
            LOGGER.info("Get server ky");
            Key key = keyStore.getKey(alias, privateKeyPassword.toCharArray());
            if (key instanceof PrivateKey) {
                return (PrivateKey) key;
            } else {
                LOGGER.error("Ks not server ky.");
                throw new LegoCheckedException("Ks not server ky.");
            }
        } catch (Exception e) {
            LOGGER.error("Get server ky error.", ExceptionUtil.getErrorMessage(e));
            throw new EmeiStorUnauthorizedException(ErrorConstant.ErrorCode.PRIVATE_KEY_FAILED);
        }
    }

    public String getCertificate() {
        return certificate;
    }

    private static void handle(Watcher.Event event) {
        LOGGER.info(String.valueOf(event));
        KeyStore keyStore = loader.getKeyStore();
        try {
            cert = keyStore.getCertificate(alias);
        } catch (KeyStoreException e) {
            throw new EmeiStorDefaultExceptionHandler("resolve public crt failed");
        }
        try {
            certificate = Base64.toBase64String(cert.getEncoded());
        } catch (CertificateEncodingException e) {
            throw new EmeiStorDefaultExceptionHandler("encode public crt failed");
        }
        publicKey = cert.getPublicKey();
    }

    /**
     * 获取 privateKeyPassword
     *
     * @return privateKeyPassword
     */
    public static String getPrivateKeyPassword() {
        return privateKeyPassword;
    }
}
