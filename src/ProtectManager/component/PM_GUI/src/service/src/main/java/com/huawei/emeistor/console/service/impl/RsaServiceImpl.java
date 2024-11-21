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

import com.huawei.emeistor.console.bean.FileSyncEntity;
import com.huawei.emeistor.console.bean.FileSyncMessage;
import com.huawei.emeistor.console.config.RedissonClientConfig;
import com.huawei.emeistor.console.contant.DeployType;
import com.huawei.emeistor.console.contant.SyncFileActionTypeEnum;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.service.RsaService;
import com.huawei.emeistor.console.util.ExceptionUtil;

import com.sun.org.apache.xerces.internal.impl.dv.util.Base64;

import lombok.extern.slf4j.Slf4j;

import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.openssl.jcajce.JcaPEMWriter;
import org.bouncycastle.util.io.pem.PemObject;
import org.redisson.api.RStream;
import org.redisson.api.RedissonClient;
import org.redisson.api.stream.StreamAddArgs;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestClientException;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.net.MalformedURLException;
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
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

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

    private static final String PRI_NAME = "rsaPrivateKey.pem";

    private static final String PUB_NAME = "rsaPublicKey.pem";

    private static final String PRI_NAME_TMP = "rsaPrivateKeyTmp.pem";

    private static final String PUB_NAME_TMP = "rsaPublicKeyTmp.pem";

    private static final String FILE_PATH = "/opt/ProtectManager";

    private static final String SYNC_FILE_TO_ALL_K8S_NODE_NAME = "syncFileToAllK8sNodeName";

    private static final String SYNC_FILE_TO_ALL_K8S_NODE_DATA_KEY = "syncFileDataKey";

    private static final int SLEEP_TIME = 1000;

    @Autowired
    private RedissonClientConfig redissonClientConfig;

    private RedissonClient redissonClient;

    /**
     * 初始化redissonClient
     */
    @PostConstruct
    public void init() {
        try {
            redissonClient = redissonClientConfig.redissonClientJsonCodec();
        } catch (InterruptedException | MalformedURLException e) {
            throw LegoCheckedException.cast(e);
        }
    }

    /**
     * 公钥生成
     *
     * @return 公钥
     * @throws NoSuchAlgorithmException 异常
     * @throws IOException io异常
     */
    @Override
    public String generateKeyPair() throws NoSuchAlgorithmException, IOException {
        KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance(ALGORITHM);
        keyPairGenerator.initialize(LEN);
        KeyPair keyPair = keyPairGenerator.generateKeyPair();
        PemObject priObject = new PemObject(PRIVATE_KEY, keyPair.getPrivate().getEncoded());
        try (JcaPEMWriter priWriter = new JcaPEMWriter(new FileWriter(FILE_PATH + SEPERATOR + PRI_NAME_TMP))) {
            priWriter.writeObject(priObject);
        }
        PemObject pubObject = new PemObject(PUBLIC_KEY, keyPair.getPublic().getEncoded());
        try (JcaPEMWriter pubWriter = new JcaPEMWriter(new FileWriter(FILE_PATH + SEPERATOR + PUB_NAME_TMP))) {
            pubWriter.writeObject(pubObject);
        }
        // 读取公钥给前端
        BufferedReader reader = null;
        StringBuilder content = new StringBuilder();
        try {
            reader = new BufferedReader(new FileReader(FILE_PATH + SEPERATOR + PUB_NAME_TMP));
            String line;
            while ((line = reader.readLine()) != null) {
                content.append(line).append("\n");
            }
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
        log.info("deploy type: {}", DeployType.getCurrentDeployType().getName());
        // 同步密钥到集群
        List<FileSyncEntity> fileSyncEntities = new ArrayList<>();
        fileSyncEntities.add(buildSyncFileEntity(FILE_PATH + SEPERATOR + PUB_NAME,
            FILE_PATH + SEPERATOR + PUB_NAME_TMP));
        fileSyncEntities.add(buildSyncFileEntity(FILE_PATH + SEPERATOR + PRI_NAME,
            FILE_PATH + SEPERATOR + PRI_NAME_TMP));
        buildAndSendMessage(fileSyncEntities, SyncFileActionTypeEnum.ADD.getType());
        if (DeployType.getCurrentDeployType().equals(DeployType.E1000)) {
            deleteFile(FILE_PATH + SEPERATOR + PUB_NAME_TMP);
            deleteFile(FILE_PATH + SEPERATOR + PRI_NAME_TMP);
        }
        return content.toString();
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
            log.info("start to decrypt");
            while (decryptBtyes == null) {
                try {
                    decryptBtyes = cipher.doFinal(Base64.decode(encrytedText));
                } catch (BadPaddingException e) {
                    log.warn("decrypt fail, try again");
                    Thread.sleep(SLEEP_TIME); // 死循环中降低CPU占用
                }
            }
        } catch (NoSuchPaddingException | NoSuchAlgorithmException | IOException | InvalidKeySpecException
                 | IllegalBlockSizeException | InvalidKeyException | InvalidAlgorithmParameterException
                 | InterruptedException e) {
            log.error("decrypt failed, cause: " + e);
            throw new RestClientException("decrypt failed");
        }
        return new String(decryptBtyes, StandardCharsets.UTF_8);
    }

    private PrivateKey getPemPrivateKey() throws IOException, NoSuchAlgorithmException, InvalidKeySpecException,
        InterruptedException {
        String filePath = FILE_PATH + SEPERATOR + PRI_NAME_TMP;
        if (DeployType.getCurrentDeployType().equals(DeployType.E1000)) {
            filePath = FILE_PATH + SEPERATOR + PRI_NAME;
        }
        File f = new File(filePath);
        FileInputStream fis = null;
        DataInputStream dis = null;
        byte[] keyBytes = new byte[0];
        while (dis == null) {
            try {
                if (!f.exists()) {
                    Thread.sleep(SLEEP_TIME);
                    continue;
                }
                fis = new FileInputStream(f);
                dis = new DataInputStream(fis);
                keyBytes = new byte[(int) f.length()];
                dis.readFully(keyBytes);
            } catch (IOException e) {
                log.error("read primary key failed, not get private key file yet");
                throw e;
            } finally {
                if (dis != null) {
                    dis.close();
                    fis.close();
                }
            }
        }
        String temp = new String(keyBytes, StandardCharsets.UTF_8);
        String privKeyPEM = temp.replace("-----BEGIN PRIVATE KEY-----\n", "");
        privKeyPEM = privKeyPEM.replace("-----END PRIVATE KEY-----", "");
        PKCS8EncodedKeySpec spec = new PKCS8EncodedKeySpec(Base64.decode(privKeyPEM));
        KeyFactory kf = KeyFactory.getInstance(ALGORITHM);
        log.info("generate private key success");
        return kf.generatePrivate(spec);
    }

    private FileSyncEntity buildSyncFileEntity(String finalPath, String tmpPath) throws IOException {
        FileSyncEntity fileEntity = new FileSyncEntity();
        fileEntity.setFileType("TEXT");
        fileEntity.setFilePath(finalPath);
        fileEntity.setFileMode("0o640");
        Path filePath = Paths.get(tmpPath);
        fileEntity.setFileContent(java.util.Base64.getEncoder().encodeToString(new String(Files
            .readAllBytes(filePath), StandardCharsets.UTF_8).getBytes(StandardCharsets.UTF_8)));
        return fileEntity;
    }

    private void deleteFile(String filePath) throws IOException {
        File filePri = new File(filePath);
        if (filePri.exists()) {
            Path pathPri = Paths.get(filePath);
            Files.delete(pathPri);
        }
    }
    private void buildAndSendMessage(List<FileSyncEntity> fileSyncEntityList, Integer action) {
        log.info("Start to send redis stream message. action: {}.", action);
        FileSyncMessage message = new FileSyncMessage();
        String requestId = UUID.randomUUID().toString();
        message.setFileSyncEntityList(fileSyncEntityList);
        message.setAction(action);
        message.setRequestId(requestId);

        RStream<Object, Object> stream = redissonClient.getStream(SYNC_FILE_TO_ALL_K8S_NODE_NAME);
        stream.add(StreamAddArgs.entry(SYNC_FILE_TO_ALL_K8S_NODE_DATA_KEY, message));
        log.info("Send redis stream message success. requestId: {}, action: {}.", requestId, action);
    }
}
