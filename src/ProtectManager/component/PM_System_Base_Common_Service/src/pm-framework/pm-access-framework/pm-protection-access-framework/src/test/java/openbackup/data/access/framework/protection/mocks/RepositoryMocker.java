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
package openbackup.data.access.framework.protection.mocks;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import com.huawei.oceanprotect.repository.s3.entity.S3Storage;
import openbackup.system.base.common.model.repository.StorageInfo;
import openbackup.system.base.common.model.repository.tape.TapeSetDetailResponse;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;

import org.apache.commons.lang3.RandomStringUtils;

import java.util.Arrays;
import java.util.UUID;

/**
 * 存储库相关数据模拟类
 *
 **/
public class RepositoryMocker {

    /**
     * 模拟S3存储对象包含认证信息
     *
     * @return 带认证信息的对象 {@code S3Storage}
     */
    public static S3Storage mockS3StorageAuthInfo() {
        S3Storage storage = new S3Storage();
        storage.setAk("aaaaaaaakkkkkkk");
        storage.setSk("ssssssskkkkkkkk");
        storage.setId(UUID.randomUUID().toString());
        return storage;
    }

    /**
     * 模拟S3存储对象包含连接信息
     *
     * @return 带连接信息的对象 {@code S3Storage}
     */
    public static S3Storage mockS3StorageEndpointInfo() {
        S3Storage storage = new S3Storage();
        storage.setId(UUID.randomUUID().toString());
        storage.setEndpoint("192.1.1.0");
        storage.setPort(443);
        return storage;
    }

    /**
     * 模拟S3存储对象包含存储库需要的全部信息
     *
     * @return 全部信息的对象 {@code S3Storage}
     */
    public static S3Storage mockS3StorageFullInfo() {
        S3Storage storage = new S3Storage();
        storage.setId(UUID.randomUUID().toString());
        storage.setEndpoint("192.1.1.0");
        storage.setPort(443);
        storage.setAk("aaaaaaaakkkkkkk");
        storage.setSk("ssssssskkkkkkkk");
        storage.setId(UUID.randomUUID().toString());
        storage.setDataBucket("/a/b/c");
        storage.setProxyEnable(true);
        storage.setProxyUrl("http://test.proxy.com");
        storage.setProxyPort("9088");
        storage.setProxyUserName("username");
        storage.setProxyUserPwd("password");
        storage.setHttps(true);
        return storage;
    }

    /**
     * 模拟本地存储对象包含认证信息
     *
     * @return 带认证信息的对象 {@code S3Storage}
     */
    public static ClusterDetailInfo mockLocalClusterInfo() {
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();
        SourceClustersParams source = new SourceClustersParams();
        source.setMgrIpList(Arrays.asList("192.168.1.3", "192.168.0.4"));
        source.setStorageDisplayIps("192.168.1.3,192.168.0.4");
        StorageSystemInfo storageSystem = new StorageSystemInfo();
        storageSystem.setStoragePort(8088);
        storageSystem.setUsername("Admin");
        storageSystem.setPassword("admin8888");
        storageSystem.setStorageEsn("XLFDI303821JND-1823");
        clusterDetailInfo.setSourceClusters(source);
        clusterDetailInfo.setStorageSystem(storageSystem);
        return clusterDetailInfo;
    }

    /**
     * 模拟本地存储对象包含连接信息
     *
     * @return 带连接信息的对象 {@code S3Storage}
     */
    public static StorageInfo mockNativeStorageEndpointInfo() {
        StorageInfo storage = new StorageInfo();
        storage.setPort(8080);
        storage.setManagementIps("127.1.1.1;99.4.100.123");
        return storage;
    }

    /**
     * 模拟本地存储对象包含存储库需要的全部信息
     *
     * @return 全部信息的对象 {@code StorageInfo}
     */
    public static StorageInfo mockNativeStorageFullInfo() {
        StorageInfo storage = new StorageInfo();
        storage.setPort(8080);
        storage.setManagementIps("127.1.1.1;99.4.100.123");
        storage.setUsername("testuser");
        storage.setPassword("testpwd");
        return storage;
    }

    /**
     * 模拟存储库基本信息
     *
     * @param id 存储库id
     * @param protocol 存储库协议枚举
     * @param type 存储库类型枚举
     * @return 存储库基本信息 {@code BaseStorageRepository}
     */
    public static BaseStorageRepository mockBaseStorageRepository(String id, RepositoryProtocolEnum protocol,
            RepositoryTypeEnum type) {
        BaseStorageRepository base = new BaseStorageRepository();
        base.setId(id);
        base.setProtocol(protocol.getProtocol());
        base.setType(type.getType());
        return base;
    }

    public static StorageRepository mockS3Repository(){
        String s3Json = "{\"id\":\"7ab74606-1aa4-4056-b85c-2a3d46baba93\",\"type\":1,\"protocol\":2,\"extendInfo\":null,\"path\":\"/a/b/c\",\"auth\":{\"authType\":4,\"authKey\":\"aaaaaaaakkkkkkk\",\"authPwd\":\"ssssssskkkkkkkk\",\"extendInfo\":null},\"endpoint\":{\"id\":\"7ab74606-1aa4-4056-b85c-2a3d46baba93\",\"ip\":\"192.1.1.0\",\"port\":443},\"proxy\":{\"port\":9088,\"userName\":\"username\",\"password\":\"password\",\"extendInfo\":null,\"enabled\":true,\"hostname\":\"http://test.proxy.com\"},\"transProtocol\":\"https\",\"local\":false,\"isLocal\":false}";
        return JSONObject.toBean(s3Json, StorageRepository.class);
    }

    public static StorageRepository mockTapRepository(){
        StorageRepository repository = new StorageRepository();
        repository.setId("f8b38f61-9c09-4c05-8caa-03b38b881242");
        repository.setType(RepositoryTypeEnum.DATA.getType());
        repository.setProtocol(RepositoryProtocolEnum.TAPE.getProtocol());
        repository.setPath("mediaSetName");
        repository.setEndpoint(new Endpoint("44e113ee-a9f5-472a-8ce6-0088fac31d1b", "Local", -1));
        repository.setLocal(Boolean.FALSE);
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.NO_AUTH);
        repository.setAuth(auth);
        return repository;
    }

    public static StorageRepository mockNativeNfsRepository(){
        String nfsJson = "{\"id\":\"44e113ee-a9f5-472a-8ce6-0088fac31d1b\",\"type\":1,\"protocol\":1,\"extendInfo\":null,\"path\":\"\",\"extendAuth\":{\"authType\":2,\"authKey\":\"Admin\",\"authPwd\":\"admin8888\",\"extendInfo\":null},\"endpoint\":{\"id\":\"44e113ee-a9f5-472a-8ce6-0088fac31d1b\",\"ip\":\"192.168.1.3,192.168.0.4\",\"port\":8088},\"proxy\":null,\"transProtocol\":\"\",\"local\":true,\"isLocal\":true}";
        return JSONObject.toBean(nfsJson, StorageRepository.class);
    }

    public static TapeSetDetailResponse mockTapeRepository(){
        final TapeSetDetailResponse tapeSetDetailResponse = new TapeSetDetailResponse();
        tapeSetDetailResponse.setMediaSetId(UUID.randomUUID().toString());
        tapeSetDetailResponse.setMediaSetName(RandomStringUtils.randomNumeric(5));
        return tapeSetDetailResponse;
    }
}
