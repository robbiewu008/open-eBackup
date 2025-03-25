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
package com.huawei.emeistor.console.service.impl;

import com.huawei.emeistor.console.contant.CommonConstant;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.kmchelp.KmcHelper;
import com.huawei.emeistor.console.service.RsaService;
import com.huawei.emeistor.console.service.SystemConfigService;
import com.huawei.emeistor.console.util.ExceptionUtil;
import com.huawei.emeistor.console.util.VerifyUtil;

import com.sun.org.apache.xerces.internal.impl.dv.util.Base64;

import lombok.extern.slf4j.Slf4j;

import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.openssl.jcajce.JcaPEMWriter;
import org.bouncycastle.util.io.pem.PemObject;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestClientException;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.StringWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Security;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.MGF1ParameterSpec;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.HashMap;
import java.util.Map;

import javax.annotation.PostConstruct;
import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.spec.OAEPParameterSpec;
import javax.crypto.spec.PSource;

/**
 * rsa公私钥对生成
 *
 */
@Slf4j
@Service
public class RsaServiceImpl implements RsaService {
    private static final String ALGORITHM = "RSA";

    private static final String PADDING = "ECB/OAEPPadding";

    private static final int LEN = 4096;

    private static final String PRIVATE_KEY = "PRIVATE KEY";

    private static final String PUBLIC_KEY = "PUBLIC KEY";

    private static final String SEPERATOR = "/";

    private static final int SLEEP_TIME = 1000;

    private static final int DECRYPT_RETRY_COUNT = 5;

    @Autowired
    private SystemConfigService systemConfigService;

    /**
     * 初始化redissonClient
     */
    @PostConstruct
    public void init() {
        try {
            generateKeyPair(false);
        } catch (Exception e) {
            log.error("Generate rsa key pair error.", ExceptionUtil.getErrorMessage(e));
        }
    }

    @Override
    public void generateKeyPair(boolean isOverwrite) {
        try {
            if (!isOverwrite && isKeyPairExist()) {
                return;
            }

            KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance(ALGORITHM);
            keyPairGenerator.initialize(LEN);
            KeyPair keyPair = keyPairGenerator.generateKeyPair();
            PemObject priObject = new PemObject(PRIVATE_KEY, keyPair.getPrivate().getEncoded());
            StringWriter privateWriter = new StringWriter();
            try (JcaPEMWriter jcaPEMWriter = new JcaPEMWriter(privateWriter)) {
                jcaPEMWriter.writeObject(priObject);
            }
            PemObject pubObject = new PemObject(PUBLIC_KEY, keyPair.getPublic().getEncoded());
            StringWriter publicWriter = new StringWriter();
            try (JcaPEMWriter jcaPEMWriter = new JcaPEMWriter(publicWriter)) {
                jcaPEMWriter.writeObject(pubObject);
            }
            // 读取公钥和私钥
            String privateContent = privateWriter.toString();
            String publicContent = publicWriter.toString();
            // 公钥和私钥加密入库
            Map<String, String> systemConfigMap = new HashMap<>();
            systemConfigMap.put(CommonConstant.PRIVATE_SYSTEM_CONFIG_KEY,
                    KmcHelper.getInstance().encrypt(privateContent));
            systemConfigMap.put(CommonConstant.PUBLIC_SYSTEM_CONFIG_KEY,
                    KmcHelper.getInstance().encrypt(publicContent));
            systemConfigService.saveOrUpdateConfig(systemConfigMap);
        } catch (NoSuchAlgorithmException | IOException e) {
            log.error("Generate rsa key pair error of algorithm or io.", ExceptionUtil.getErrorMessage(e));
            throw LegoCheckedException.cast(e);
        }
    }

    private boolean isKeyPairExist() {
        String privateContent = systemConfigService.queryConfig(CommonConstant.PRIVATE_SYSTEM_CONFIG_KEY, true);
        String publicContent = systemConfigService.queryConfig(CommonConstant.PUBLIC_SYSTEM_CONFIG_KEY, true);
        return !VerifyUtil.isEmpty(privateContent) && !VerifyUtil.isEmpty(publicContent);
    }


    private String readFile(String path) throws IOException {
        BufferedReader reader = null;
        StringBuilder content = new StringBuilder();
        try {
            reader = new BufferedReader(new FileReader(path));
            String line;
            while ((line = reader.readLine()) != null) {
                content.append(line).append("\n");
            }
            return content.toString();
        } catch (IOException e) {
            log.error("read public key failed");
            throw e;
        } finally {
            if (reader != null) {
                try {
                    reader.close();
                } catch (IOException e) {
                    log.error("close reader failed: ", ExceptionUtil.getErrorMessage(e));
                }
            }
        }
    }


    /**
     * 解密操作
     *
     * @param encrytedText 加密内容
     * @return 解密内容
     */
    @Override
    public String decrypt(String encrytedText) {
        byte[] decryptBtyes = null;
        try {
            Security.addProvider(new BouncyCastleProvider());
            OAEPParameterSpec oaepParameterSpec = new OAEPParameterSpec("SHA-256", "MGF1",
                MGF1ParameterSpec.SHA256, PSource.PSpecified.DEFAULT);
            Cipher cipher = Cipher.getInstance(ALGORITHM + SEPERATOR + PADDING);
            cipher.init(Cipher.DECRYPT_MODE, getPemPrivateKey(), oaepParameterSpec);
            decryptBtyes = getDecryptBytes(cipher, encrytedText);
        } catch (NoSuchPaddingException | NoSuchAlgorithmException | IOException | InvalidKeySpecException
                 | IllegalBlockSizeException | InvalidKeyException | InvalidAlgorithmParameterException
                 | InterruptedException e) {
            log.error("decrypt failed, cause: ", ExceptionUtil.getErrorMessage(e));
            throw new RestClientException("decrypt failed");
        }
        return new String(decryptBtyes, StandardCharsets.UTF_8);
    }

    private byte[] getDecryptBytes(Cipher cipher, String encrytedText) throws InterruptedException,
            IllegalBlockSizeException {
        byte[] decryptBtyes = null;
        int count = 1;
        while (decryptBtyes == null) {
            try {
                decryptBtyes = cipher.doFinal(Base64.decode(encrytedText));
            } catch (BadPaddingException e) {
                log.warn("Decrypt fail, try again", ExceptionUtil.getErrorMessage(e));
                if (count > DECRYPT_RETRY_COUNT) {
                    throw LegoCheckedException.cast(e);
                }
                count++;
                Thread.sleep(SLEEP_TIME); // 死循环中降低CPU占用
            }
        }
        return decryptBtyes;
    }

    private PrivateKey getPemPrivateKey() throws IOException, NoSuchAlgorithmException, InvalidKeySpecException,
        InterruptedException {
        String temp = systemConfigService.queryConfig(CommonConstant.PRIVATE_SYSTEM_CONFIG_KEY, true);
        String privKeyPEM = temp.replace("-----BEGIN PRIVATE KEY-----\n", "");
        privKeyPEM = privKeyPEM.replace("-----END PRIVATE KEY-----", "");
        PKCS8EncodedKeySpec spec = new PKCS8EncodedKeySpec(Base64.decode(privKeyPEM));
        KeyFactory kf = KeyFactory.getInstance(ALGORITHM);
        return kf.generatePrivate(spec);
    }

    private void deleteFile(String filePath) {
        File filePri = new File(filePath);
        if (filePri.exists()) {
            Path pathPri = Paths.get(filePath);
            try {
                Files.delete(pathPri);
            } catch (IOException e) {
                log.error("delete file failed, filePath is: {}", filePath, ExceptionUtil.getErrorMessage(e));
            }
        }
    }

    @Override
    public String queryPublicKey() {
        String publicKey = systemConfigService.queryConfig(CommonConstant.PUBLIC_SYSTEM_CONFIG_KEY, true);
        if (!VerifyUtil.isEmpty(publicKey)) {
            return publicKey;
        }
        // 如果查询没有公钥，则重新生成一次
        generateKeyPair(true);
        return systemConfigService.queryConfig(CommonConstant.PUBLIC_SYSTEM_CONFIG_KEY, true);
    }
}
