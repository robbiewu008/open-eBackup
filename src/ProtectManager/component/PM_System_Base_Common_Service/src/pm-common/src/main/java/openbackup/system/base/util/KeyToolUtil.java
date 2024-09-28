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
package openbackup.system.base.util;

import openbackup.system.base.sdk.kmc.EncryptorRestApi;
import openbackup.system.base.sdk.kmc.model.CiphertextVo;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.io.FileUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

/**
 * KeyStore工具类
 *
 */
@Slf4j
@Component
public class KeyToolUtil {
    /**
     * SSL版本
     */
    public static final String SSL_CONTEXT_VERSION = "TLSv1.3";

    /**
     * KeyStore密钥文件
     */
    @Value("${server.ssl.key-store-password-file}")
    private String keyStorePwdFile;

    /**
     * KeyStore类型
     */
    @Value("${server.ssl.key-store-type}")
    private String type;

    /**
     * KeyStore中存放的双向证书别名
     */
    @Value("${server.ssl.key-alias}")
    private String alias;

    /**
     * KeyStore文件
     */
    @Value("${server.ssl.key-store}")
    private String keyStoreFile;

    @Autowired
    private EncryptorRestApi encryptorRestApi;

    /**
     * 从文件中读取密钥并解密为明文
     *
     * @param keyStoreAuthFile 指定密码文件
     * @return keystore密码
     */
    public String getKeyStorePassword(String keyStoreAuthFile) {
        String cipherText = null;
        try {
            cipherText = FileUtils.readFileToString(new File(keyStoreAuthFile), StandardCharsets.UTF_8);
        } catch (IOException e) {
            log.error("read keystore auth file error: ", e);
        }
        CiphertextVo ciphertextVo = new CiphertextVo();
        ciphertextVo.setCiphertext(cipherText);
        return encryptorRestApi.decrypt(ciphertextVo).getPlaintext();
    }

    /**
     * 生成KeyStore
     *
     * @param storePwd KeyStore文件密钥
     * @param storePath KeyStore文件路径
     */
    public static void genKeyStore(String storePwd, String storePath) {
        try (FileOutputStream fos = new FileOutputStream(storePath)) {
            KeyStore keyStore = KeyStore.getInstance("PKCS12");
            keyStore.load(null, storePwd.toCharArray());
            keyStore.store(fos, storePwd.toCharArray());
        } catch (IOException | GeneralSecurityException e) {
            log.error("generator keystore failed");
        }
    }

    /**
     * KeyStore操作公共接口
     *
     */
    private interface KeyStoreConsumer {
        /**
         * KeyStore处理方法
         *
         * @param keyStore keyStore实例
         * @throws KeyStoreException 处理KeyStore异常
         */
        void accept(KeyStore keyStore) throws KeyStoreException;
    }

    /**
     * 加载内部微服务认证Keystore
     *
     * @return 内部船务认证Keystore
     */
    public KeyStore getInternalKeystore() {
        return loadKeystore(getKeyStorePassword(keyStorePwdFile), keyStoreFile);
    }

    /**
     * 加载KeyStore文件并返回实例
     *
     * @param password KeyStore文件密钥
     * @param path KeyStore文件路径
     * @return KeyStore实例
     */
    @ExterAttack
    public KeyStore loadKeystore(String password, String path) {
        KeyStore keyStore = null;
        try (InputStream ins = new FileInputStream(path)) {
            keyStore = KeyStore.getInstance(type);
            keyStore.load(ins, password.toCharArray());
        } catch (IOException | KeyStoreException | NoSuchAlgorithmException | CertificateException e) {
            log.error("load keystore file error: ", e);
        }
        return keyStore;
    }

    private void keystoreHandle(String pwd, String storePath, KeyStoreConsumer consumer) {
        KeyStore keyStore = loadKeystore(pwd, storePath);
        try {
            consumer.accept(keyStore);
        } catch (KeyStoreException e) {
            log.error("handle keystore error: ", e);
        }
        try (FileOutputStream fos = new FileOutputStream(storePath)) {
            keyStore.store(fos, pwd.toCharArray());
        } catch (IOException | KeyStoreException | NoSuchAlgorithmException | CertificateException e) {
            log.error("store keystore error: ", e);
        }
    }

    /**
     * 更新KeyStore中的证书文件
     *
     * @param pwd KeyStore密钥
     * @param x509Certificate 读取证书
     * @param storePath KeyStore文件路径
     * @param privateKey 证书私钥
     * @param privatePwd 证书私钥密码
     */
    public void updateTokenEntry(String pwd, X509Certificate x509Certificate, String storePath,
        PrivateKey privateKey, String privatePwd) {
        keystoreHandle(pwd, storePath, keyStore -> {
            keyStore.deleteEntry(alias);
            X509Certificate[] chain = new X509Certificate[1];
            chain[0] = x509Certificate;
            keyStore.setKeyEntry(alias, privateKey, privatePwd.toCharArray(), chain);
        });
    }

    /**
     * 写入KeyStore中的证书文件
     *
     * @param pwd KeyStore密钥
     * @param x509Certificate 读取证书
     * @param storePath KeyStore文件路径
     * @param privateKey 证书私钥
     * @param privatePwd 证书私钥密码
     */
    public void insertStoreEntry(String pwd, X509Certificate x509Certificate, String storePath,
        PrivateKey privateKey, String privatePwd) {
        keystoreHandle(pwd, storePath, keyStore -> {
            X509Certificate[] chain = new X509Certificate[1];
            chain[0] = x509Certificate;
            keyStore.setKeyEntry(alias, privateKey, privatePwd.toCharArray(), chain);
        });
    }

    /**
     * 导入证书
     *
     * @param pwd 证书密钥
     * @param x509Certificate 认证
     * @param storePath 证书路径
     * @param aliasKey 证书别名
     */
    public void importCert(String pwd, X509Certificate x509Certificate, String storePath, String aliasKey) {
        keystoreHandle(pwd, storePath, keyStore -> keyStore.setCertificateEntry(aliasKey, x509Certificate));
    }
}
