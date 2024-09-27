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
package openbackup.data.access.framework.protection.service.repository;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.notNullValue;
import static org.hamcrest.Matchers.nullValue;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;

import openbackup.data.access.framework.protection.mocks.RepositoryMocker;

import openbackup.data.access.framework.protection.service.repository.strategies.NativeCifsRepositoryStrategy;
import openbackup.data.access.framework.protection.service.repository.strategies.NativeNfsRepositoryStrategy;
import openbackup.data.access.framework.protection.service.repository.strategies.RepositoryStrategy;
import openbackup.data.access.framework.protection.service.repository.strategies.S3RepositoryStrategy;
import openbackup.data.access.framework.protection.service.repository.strategies.TapeRepositoryStrategy;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import com.huawei.oceanprotect.repository.s3.entity.S3Storage;
import com.huawei.oceanprotect.repository.s3.service.S3StorageService;
import com.huawei.oceanprotect.repository.tapelibrary.service.MediaSetService;
import com.huawei.oceanprotect.system.base.cert.entity.ObjectCertInfo;
import com.huawei.oceanprotect.system.base.cert.service.ObjectCertDependencyService;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.repository.tape.TapeSetDetailResponse;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.ClustersInfoVo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.session.IStorageDeviceRepository;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.StorageDevice;

import com.google.common.collect.Lists;

import org.apache.hc.core5.http.URIScheme;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * 存储库策略类的测试集合
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/8
 **/
public class RepositoryStrategyManagerTest {
    @Rule
    public final ExpectedException exception = ExpectedException.none();

    private RepositoryStrategyManager repositoryStrategyManager;

    private final ObjectCertDependencyService objectCertDependencyService = Mockito.mock(ObjectCertDependencyService.class);

    private S3StorageService s3StorageService;

    private ClusterNativeApi clusterNativeApi;

    private MediaSetService mediaSetService;

    private IStorageDeviceRepository repository;

    @Before
    public void initMock() throws Exception{
        clusterNativeApi = mock(ClusterNativeApi.class);
        s3StorageService = mock(S3StorageService.class);
        mediaSetService = mock(MediaSetService.class);
        repository = mock(IStorageDeviceRepository.class);
        Map<String, RepositoryStrategy> strategyMap = new HashMap<>();
        strategyMap.put("s3RepositoryStrategy", new S3RepositoryStrategy(s3StorageService, objectCertDependencyService));
        NativeCifsRepositoryStrategy nativeCifsRepositoryStrategy = new NativeCifsRepositoryStrategy(clusterNativeApi,
            repository);
        NativeNfsRepositoryStrategy nativeNfsRepositoryStrategy = new NativeNfsRepositoryStrategy(clusterNativeApi,
            repository);
        strategyMap.put("nativeCifsRepositoryStrategy", nativeCifsRepositoryStrategy);
        strategyMap.put("nativeNfsRepositoryStrategy", nativeNfsRepositoryStrategy);
        strategyMap.put("tapeRepositoryStrategy", new TapeRepositoryStrategy(mediaSetService));
        repositoryStrategyManager = new RepositoryStrategyManager(strategyMap);
        PowerMockito.doNothing().when(objectCertDependencyService).insert(Mockito.any(), Mockito.any(), Mockito.any());
        mockStorage();
        mockClusterVO();
    }

    private void mockClusterVO() {
        ClustersInfoVo clustersInfoVo = new ClustersInfoVo();
        clustersInfoVo.setStorageEsn("XLFDI303821JND-1823");
        given(clusterNativeApi.getCurrentClusterVoInfo()).willReturn(clustersInfoVo);
        given(clusterNativeApi.queryCurrentGroupManageIpList()).willReturn(Arrays.asList("192.168.1.3", "192.168.0.4"));
    }

    private void mockStorage() {
        StorageDevice storageDevice = new StorageDevice();
        storageDevice.setUserName("Admin");
        storageDevice.setPassword("admin8888");
        given(repository.findLocalStorage(true)).willReturn(storageDevice);
    }

    /**
     * 用例名称：验证策略管理器能够正确获取NATIVE的策略类<br/>
     * 前置条件：无<br/>
     * check点：返回的RepositoryStrategy实例类型为NativeRepositoryStrategy<br/>
     */
    @Test
    public void should_return_NativeRepositoryStrategy_when_getStrategy_given_NATIVE_NFS() {
        // Given
        final RepositoryProtocolEnum nfsProtocol = RepositoryProtocolEnum.NATIVE_NFS;
        final RepositoryProtocolEnum cifsProtocol = RepositoryProtocolEnum.NATIVE_CIFS;
        // When
        final RepositoryStrategy nfsStrategy = repositoryStrategyManager.getStrategy(nfsProtocol);
        final RepositoryStrategy cifsStrategy = repositoryStrategyManager.getStrategy(cifsProtocol);
        // Then
        assertThat(nfsStrategy, instanceOf(NativeNfsRepositoryStrategy.class));
        assertThat(cifsStrategy, instanceOf(NativeCifsRepositoryStrategy.class));
    }

    /**
     * 用例名称：验证正确查询本地存储的认证信息<br/>
     * 前置条件：无<br/>
     * check点：返回信息不为空，并且对应的字段属性符合预期<br/>
     */
    @Test
    public void should_return_Authentication_when_getAuthentication_given_NATIVE_NFS_and_storage_exist() {
        // Given
        final RepositoryProtocolEnum nativeProtocol = RepositoryProtocolEnum.NATIVE_NFS;
        final ClusterDetailInfo clusterDetailInfo = RepositoryMocker.mockLocalClusterInfo();
        final String storageId = UUID.randomUUID().toString();
        given(clusterNativeApi.queryDecryptCurrentGroupClusterDetails()).willReturn(clusterDetailInfo);
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(nativeProtocol);
        final Authentication latestAuthentication = strategy.getAuthentication(storageId);
        // Then
        assertThat(latestAuthentication, is(notNullValue()));
        assertThat(latestAuthentication.getAuthKey(), is(clusterDetailInfo.getStorageSystem().getUsername()));
        assertThat(latestAuthentication.getAuthPwd(), is(clusterDetailInfo.getStorageSystem().getPassword()));
    }

    /**
     * 用例名称：验证正确查询本地存储的连接信息<br/>
     * 前置条件：无<br/>
     * check点：返回信息不为空，并且对应的字段属性符合预期<br/>
     */
    @Test
    public void should_return_Endpoint_when_getEndpoint_given_NATIVE_NFS_and_storage_exist() {
        // Given
        final RepositoryProtocolEnum nativeProtocol = RepositoryProtocolEnum.NATIVE_NFS;
        final ClusterDetailInfo clusterDetailInfo = RepositoryMocker.mockLocalClusterInfo();
        final String storageId = UUID.randomUUID().toString();
        given(clusterNativeApi.queryDecryptCurrentGroupClusterDetails()).willReturn(clusterDetailInfo);
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(nativeProtocol);
        final Endpoint latestEndpoint = strategy.getEndpoint(storageId);
        // Then
        assertThat(latestEndpoint, is(notNullValue()));
        assertThat(latestEndpoint.getIp(), is(clusterDetailInfo.getSourceClusters().getStorageDisplayIps()));
    }

    /**
     * 用例名称：验证正确查询本地存储的认证信息<br/>
     * 前置条件：无<br/>
     * check点：返回信息不为空，并且对应的字段属性符合预期<br/>
     */
    @Test
    public void should_return_StorageRepository_when_getRepository_given_NATIVE_NFS_and_storage_exist() {
        // Given
        final RepositoryProtocolEnum nativeProtocol = RepositoryProtocolEnum.NATIVE_NFS;
        final ClusterDetailInfo clusterDetailInfo = RepositoryMocker.mockLocalClusterInfo();
        final String storageId = UUID.randomUUID().toString();
        final int authType = Authentication.APP_PASSWORD;
        final RepositoryTypeEnum typeEnum = RepositoryTypeEnum.DATA;
        given(clusterNativeApi.queryDecryptCurrentGroupClusterDetails()).willReturn(clusterDetailInfo);
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(nativeProtocol);
        BaseStorageRepository baseStorageRepository = new BaseStorageRepository(storageId, typeEnum.getType(),
            nativeProtocol.getProtocol(), null);
        given(clusterNativeApi.queryTargetClusterListDetails(Mockito.any())).willReturn(Lists.newArrayList(clusterDetailInfo));
        final StorageRepository repository = strategy.getRepository(baseStorageRepository);
        // Then
        assertThat(repository, is(notNullValue()));
        assertThat(repository.getProtocol(), is(RepositoryProtocolEnum.NFS.getProtocol()));
        assertThat(repository.getType(), is(typeEnum.getType()));
        assertThat(repository.getLocal(), is(Boolean.TRUE));
        assertThat(repository.getPath(), is(nullValue()));
        assertThat(repository.getExtendAuth().getAuthKey(), is(clusterDetailInfo.getStorageSystem().getUsername()));
        assertThat(repository.getExtendAuth().getAuthPwd(), is(clusterDetailInfo.getStorageSystem().getPassword()));
        assertThat(repository.getExtendAuth().getAuthType(), is(authType));
        assertThat(repository.getEndpoint().getId(), is(storageId));
        assertThat(repository.getEndpoint().getIp(), is(clusterDetailInfo.getSourceClusters().getStorageDisplayIps()));
    }

    /**
     * 用例名称：验证正确查询本地存储的认证信息<br/>
     * 前置条件：无<br/>
     * check点：返回信息不为空，并且对应的字段属性符合预期<br/>
     */
    @Test
    public void should_return_Authentication_when_getAuthentication_given_NATIVE_CIFS_and_storage_exist() {
        // Given
        final RepositoryProtocolEnum nativeProtocol = RepositoryProtocolEnum.NATIVE_CIFS;
        final ClusterDetailInfo clusterDetailInfo = RepositoryMocker.mockLocalClusterInfo();
        final String storageId = UUID.randomUUID().toString();
        given(clusterNativeApi.queryDecryptCurrentGroupClusterDetails()).willReturn(clusterDetailInfo);
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(nativeProtocol);
        final Authentication latestAuthentication = strategy.getAuthentication(storageId);
        // Then
        assertThat(latestAuthentication, is(notNullValue()));
        assertThat(latestAuthentication.getAuthKey(), is(clusterDetailInfo.getStorageSystem().getUsername()));
        assertThat(latestAuthentication.getAuthPwd(), is(clusterDetailInfo.getStorageSystem().getPassword()));
    }

    /**
     * 用例名称：验证正确查询本地存储的连接信息<br/>
     * 前置条件：无<br/>
     * check点：返回信息不为空，并且对应的字段属性符合预期<br/>
     */
    @Test
    public void should_return_Endpoint_when_getEndpoint_given_NATIVE_CIFS_and_storage_exist() {
        // Given
        final RepositoryProtocolEnum nativeProtocol = RepositoryProtocolEnum.NATIVE_CIFS;
        final ClusterDetailInfo clusterDetailInfo = RepositoryMocker.mockLocalClusterInfo();
        final String storageId = UUID.randomUUID().toString();
        given(clusterNativeApi.queryDecryptCurrentGroupClusterDetails()).willReturn(clusterDetailInfo);
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(nativeProtocol);
        final Endpoint latestEndpoint = strategy.getEndpoint(storageId);
        // Then
        assertThat(latestEndpoint, is(notNullValue()));
        assertThat(latestEndpoint.getIp(), is(clusterDetailInfo.getSourceClusters().getStorageDisplayIps()));
    }

    /**
     * 用例名称：验证正确查询本地存储的认证信息<br/>
     * 前置条件：无<br/>
     * check点：返回信息不为空，并且对应的字段属性符合预期<br/>
     */
    @Test
    public void should_return_StorageRepository_when_getRepository_given_NATIVE_CIFS_and_storage_exist() {
        // Given
        final RepositoryProtocolEnum nativeProtocol = RepositoryProtocolEnum.NATIVE_CIFS;
        final ClusterDetailInfo clusterDetailInfo = RepositoryMocker.mockLocalClusterInfo();
        final String storageId = UUID.randomUUID().toString();
        final int authType = Authentication.APP_PASSWORD;
        final RepositoryTypeEnum typeEnum = RepositoryTypeEnum.DATA;
        given(clusterNativeApi.queryDecryptCurrentGroupClusterDetails()).willReturn(clusterDetailInfo);
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(nativeProtocol);
        BaseStorageRepository baseStorageRepository = new BaseStorageRepository(storageId, typeEnum.getType(),
            nativeProtocol.getProtocol(), null);
        final StorageRepository repository = strategy.getRepository(baseStorageRepository);
        // Then
        assertThat(repository, is(notNullValue()));
        assertThat(repository.getProtocol(), is(RepositoryProtocolEnum.CIFS.getProtocol()));
        assertThat(repository.getType(), is(typeEnum.getType()));
        assertThat(repository.getLocal(), is(Boolean.TRUE));
        assertThat(repository.getPath(), is(nullValue()));
        assertThat(repository.getExtendAuth().getAuthKey(), is(clusterDetailInfo.getStorageSystem().getUsername()));
        assertThat(repository.getExtendAuth().getAuthPwd(), is(clusterDetailInfo.getStorageSystem().getPassword()));
        assertThat(repository.getExtendAuth().getAuthType(), is(authType));
        assertThat(repository.getEndpoint().getId(), is(storageId));
        assertThat(repository.getEndpoint().getIp(), is(clusterDetailInfo.getSourceClusters().getStorageDisplayIps()));
    }

    /**
     * 用例名称：验证策略管理器能够正确获取S3的策略类<br/>
     * 前置条件：无<br/>
     * check点：返回的RepositoryStrategy实例类型为S3RepositoryStrategy<br/>
     */
    @Test
    public void should_return_S3RepositoryStrategy_when_getStrategy_given_S3_strategy() {
        // Given
        final RepositoryProtocolEnum nativeProtocol = RepositoryProtocolEnum.S3;
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(nativeProtocol);
        // Then
        assertThat(strategy, instanceOf(S3RepositoryStrategy.class));
    }

    /**
     * 用例名称：验证正确查询S3存储的认证信息<br/>
     * 前置条件：无<br/>
     * check点：返回信息不为空，并且对应的字段属性符合预期<br/>
     */
    @Test
    public void should_return_Authentication_when_getAuthentication_given_S3_and_storage_exist() {
        // Given
        final RepositoryProtocolEnum nativeProtocol = RepositoryProtocolEnum.S3;
        final S3Storage mockS3 = RepositoryMocker.mockS3StorageAuthInfo();
        final String storageId = mockS3.getId();
        given(s3StorageService.queryS3Storage(eq(storageId))).willReturn(mockS3);
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(nativeProtocol);
        final Authentication latestAuthentication = strategy.getAuthentication(storageId);
        // Then
        assertThat(latestAuthentication, is(notNullValue()));
        assertThat(latestAuthentication.getAuthKey(), is(mockS3.getAk()));
        assertThat(latestAuthentication.getAuthPwd(), is(mockS3.getSk()));
    }

    /**
     * 用例名称：验证正确查询S3存储的存储库信息，不包含代理信息<br/>
     * 前置条件：无<br/>
     * check点：返回信息不为空，并且对应的字段属性符合预期<br/>
     */
    @Test
    public void should_return_StorageRepository_when_getRepository_given_S3_and_storage_exist_and_proxy_not_enable() {
        // Given
        final RepositoryProtocolEnum s3Protocol = RepositoryProtocolEnum.S3;
        final S3Storage mockS3 = RepositoryMocker.mockS3StorageFullInfo();
        mockS3.setProxyEnable(false);
        final String storageId = mockS3.getId();
        final int authType = Authentication.AKSK;
        final RepositoryTypeEnum typeEnum = RepositoryTypeEnum.DATA;
        given(s3StorageService.queryS3Storage(eq(storageId))).willReturn(mockS3);
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(s3Protocol);
        BaseStorageRepository baseStorageRepository = new BaseStorageRepository(storageId, typeEnum.getType(),
            s3Protocol.getProtocol(), null);
        ObjectCertInfo objectCertInfo = new ObjectCertInfo();
        objectCertInfo.setCertName("cert1");
        objectCertInfo.setCertId("certId");
        PowerMockito.when(objectCertDependencyService.getCertNameByObjectId(Mockito.any())).thenReturn(objectCertInfo);
        final StorageRepository repository = strategy.getRepository(baseStorageRepository);
        // Then
        assertThat(repository, is(notNullValue()));
        assertThat(repository.getProtocol(), is(s3Protocol.getProtocol()));
        assertThat(repository.getType(), is(typeEnum.getType()));
        assertThat(repository.getLocal(), is(Boolean.FALSE));
        assertThat(repository.getPath(), is(mockS3.getDataBucket()));
        assertThat(repository.getAuth().getAuthKey(), is(mockS3.getAk()));
        assertThat(repository.getAuth().getAuthPwd(), is(mockS3.getSk()));
        assertThat(repository.getAuth().getAuthType(), is(authType));
        assertThat(repository.getAuth().getExtendInfo().get("certName"), is("cert1"));
        assertThat(repository.getEndpoint().getId(), is(mockS3.getId()));
        assertThat(repository.getEndpoint().getIp(), is(mockS3.getEndpoint()));
        assertThat(repository.getEndpoint().getPort(), is(mockS3.getPort()));
        assertThat(repository.getProxy(), is(notNullValue()));
        assertThat(repository.getProxy().isEnabled(), is(false));
        assertThat(repository.getTransProtocol(), is(URIScheme.HTTPS.id));
    }

    /**
     * 用例名称：验证正确查询S3存储的连接信息<br/>
     * 前置条件：无<br/>
     * check点：返回信息不为空，并且对应的字段属性符合预期<br/>
     */
    @Test
    public void should_return_Endpoint_when_getEndpoint_given_S3_and_storage_exist() {
        // Given
        final RepositoryProtocolEnum nativeProtocol = RepositoryProtocolEnum.S3;
        final S3Storage mockS3 = RepositoryMocker.mockS3StorageEndpointInfo();
        final String storageId = mockS3.getId();
        given(s3StorageService.queryS3Storage(eq(storageId))).willReturn(mockS3);
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(nativeProtocol);
        final Endpoint latestEndpoint = strategy.getEndpoint(storageId);
        // Then
        assertThat(latestEndpoint, is(notNullValue()));
        assertThat(latestEndpoint.getIp(), is(mockS3.getEndpoint()));
        assertThat(latestEndpoint.getPort(), is(mockS3.getPort()));
    }

    /**
     * 用例名称：验证正确查询S3存储时存储信息不存在<br/>
     * 前置条件：无<br/>
     * check点：抛出异常符合预期<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_getEndpoint_given_S3_and_storage_not_exist() {
        // Given
        final RepositoryProtocolEnum nativeProtocol = RepositoryProtocolEnum.S3;
        final String storageId = UUID.randomUUID().toString();
        given(s3StorageService.queryS3Storage(eq(storageId))).willReturn(null);
        // Then
        exception.expect(LegoCheckedException.class);
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(nativeProtocol);
        strategy.getEndpoint(storageId);
    }

    /**
     * 用例名称：验证正确查询S3存储的存储库信息，包含代理信息<br/>
     * 前置条件：无<br/>
     * check点：返回信息不为空，并且对应的字段属性符合预期<br/>
     */
    @Test
    public void should_return_StorageRepository_when_getRepository_given_S3_and_storage_exist_and_proxy_enable() {
        // Given
        final RepositoryProtocolEnum s3Protocol = RepositoryProtocolEnum.S3;
        final S3Storage mockS3 = RepositoryMocker.mockS3StorageFullInfo();
        final String storageId = mockS3.getId();
        final int authType = Authentication.AKSK;
        final RepositoryTypeEnum typeEnum = RepositoryTypeEnum.DATA;
        ObjectCertInfo objectCertInfo = new ObjectCertInfo();
        objectCertInfo.setCertName("cert1");
        objectCertInfo.setCertId("certId");
        PowerMockito.when(objectCertDependencyService.getCertNameByObjectId(Mockito.any())).thenReturn(objectCertInfo);
        given(s3StorageService.queryS3Storage(eq(storageId))).willReturn(mockS3);
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(s3Protocol);
        BaseStorageRepository baseStorageRepository = new BaseStorageRepository(storageId, typeEnum.getType(),
            s3Protocol.getProtocol(), null);
        final StorageRepository repository = strategy.getRepository(baseStorageRepository);
        // Then
        assertThat(repository, is(notNullValue()));
        assertThat(repository.getProtocol(), is(s3Protocol.getProtocol()));
        assertThat(repository.getType(), is(typeEnum.getType()));
        assertThat(repository.getLocal(), is(Boolean.FALSE));
        assertThat(repository.getPath(), is(mockS3.getDataBucket()));
        assertThat(repository.getAuth().getAuthKey(), is(mockS3.getAk()));
        assertThat(repository.getAuth().getAuthPwd(), is(mockS3.getSk()));
        assertThat(repository.getAuth().getAuthType(), is(authType));
        assertThat(repository.getAuth().getExtendInfo().get("certName"), is("cert1"));
        assertThat(repository.getEndpoint().getId(), is(mockS3.getId()));
        assertThat(repository.getEndpoint().getIp(), is(mockS3.getEndpoint()));
        assertThat(repository.getEndpoint().getPort(), is(mockS3.getPort()));
        assertThat(repository.getProxy(), is(notNullValue()));
        assertThat(repository.getProxy().getHostName(), is(mockS3.getProxyUrl()));
        assertThat(repository.getProxy().getPort(), is(Integer.parseInt(mockS3.getProxyPort())));
        assertThat(repository.getProxy().getUserName(), is(mockS3.getProxyUserName()));
        assertThat(repository.getProxy().getPassword(), is(mockS3.getProxyUserPwd()));
        assertThat(repository.getProxy().getExtendInfo(), is(nullValue()));
    }

    /**
     * 用例名称：验证正确查询磁带库认证信息<br/>
     * 前置条件：无<br/>
     * check点：返回信息不为空，并且对应的字段属性符合预期<br/>
     */
    @Test
    public void should_return_StorageRepository_when_getAuthentication_given_TAPE_and_storage_exist() {
        // Given
        final RepositoryProtocolEnum tapeProtocol = RepositoryProtocolEnum.TAPE;
        final String storageId = UUID.randomUUID().toString();
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(tapeProtocol);
        final Authentication authentication = strategy.getAuthentication(storageId);
        // Then
        assertThat(authentication, is(notNullValue()));
        assertThat(authentication.getAuthType(), is(Authentication.NO_AUTH));
    }

    /**
     * 用例名称：验证正确查询磁带库认证信息<br/>
     * 前置条件：无<br/>
     * check点：返回信息不为空，并且对应的字段属性符合预期<br/>
     */
    @Test
    public void should_return_StorageRepository_when_getEndpoint_given_TAPE_and_storage_exist() {
        // Given
        final RepositoryProtocolEnum tapeProtocol = RepositoryProtocolEnum.TAPE;
        final String storageId = UUID.randomUUID().toString();
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(tapeProtocol);
        final Endpoint endpoint = strategy.getEndpoint(storageId);
        // Then
        assertThat(endpoint, is(notNullValue()));
        assertThat(endpoint.getId(), is(nullValue()));
        assertThat(endpoint.getIp(), is(nullValue()));
        assertThat(endpoint.getPort(), is(0));
    }

    /**
     * 用例名称：验证正确查询磁带库存储信息<br/>
     * 前置条件：无<br/>
     * check点：返回信息不为空，并且对应的字段属性符合预期<br/>
     */
    @Test
    public void should_return_StorageRepository_when_getRepository_given_TAPE_and_storage_exist() {
        // Given
        final RepositoryProtocolEnum tapeProtocol = RepositoryProtocolEnum.TAPE;
        final String storageId = UUID.randomUUID().toString();
        final RepositoryTypeEnum typeEnum = RepositoryTypeEnum.DATA;
        final TapeSetDetailResponse tapeSetDetailResponse = RepositoryMocker.mockTapeRepository();
        tapeSetDetailResponse.setMediaSetId(storageId);
        tapeSetDetailResponse.setMediaSetName("test");
        given(mediaSetService.getTapeSetDetail(eq(storageId))).willReturn(tapeSetDetailResponse);
        // When
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(tapeProtocol);
        BaseStorageRepository baseStorageRepository = new BaseStorageRepository(storageId, typeEnum.getType(),
            tapeProtocol.getProtocol(), null);
        final StorageRepository repository = strategy.getRepository(baseStorageRepository);
        // Then
        assertThat(repository, is(notNullValue()));
        assertThat(repository.getProtocol(), is(RepositoryProtocolEnum.TAPE.getProtocol()));
        assertThat(repository.getType(), is(typeEnum.getType()));
        assertThat(repository.getLocal(), is(Boolean.FALSE));
        assertThat(repository.getPath(), is(tapeSetDetailResponse.getMediaSetName()));
        assertThat(repository.getAuth(), is(notNullValue()));
        assertThat(repository.getEndpoint().getIp(), is("test"));
        assertThat(repository.getExtendInfo(), is(nullValue()));
    }
}
