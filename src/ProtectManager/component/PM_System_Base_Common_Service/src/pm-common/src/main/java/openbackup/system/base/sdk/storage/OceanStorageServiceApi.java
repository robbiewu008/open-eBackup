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
package openbackup.system.base.sdk.storage;

import openbackup.system.base.sdk.storage.model.HyperMetroDomainBo;
import openbackup.system.base.sdk.storage.model.HyperMetroPairBo;
import openbackup.system.base.sdk.storage.model.OceanStorageLogincalPortRes;
import openbackup.system.base.sdk.storage.model.OceanStorageLunInfoRes;
import openbackup.system.base.sdk.storage.model.OceanStorageSession;
import openbackup.system.base.sdk.storage.model.OceanStorageSystemInfoRes;
import openbackup.system.base.sdk.storage.model.StorageAlarmObj;
import openbackup.system.base.sdk.storage.model.StorageAlarmResponse;
import openbackup.system.base.sdk.storage.model.StorageCommonRes;
import openbackup.system.base.sdk.storage.model.StorageCountResponse;
import openbackup.system.base.sdk.storage.model.StorageFileSystemBo;
import openbackup.system.base.sdk.storage.model.StorageFileSystemSnapshotBo;
import openbackup.system.base.sdk.storage.model.StorageRemoteReplicationPairBo;
import openbackup.system.base.sdk.storage.model.StorageSessionReq;
import openbackup.system.base.security.exterattack.ExterAttack;

import feign.Headers;
import feign.Param;
import feign.RequestLine;

import org.springframework.web.bind.annotation.RequestBody;

import java.net.URI;
import java.util.List;

/**
 * dorado 服务接口定义
 *
 * @author p00511147
 * @since 2020/10/29
 */
public interface OceanStorageServiceApi {
    /**
     * 查询设备信息
     *
     * @param url 查询设备url
     * @param iBaseToken 需要携带的会话iBaseToken
     * @param cookie cookie
     * @return 系统信息
     */
    @ExterAttack
    @RequestLine("GET")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    StorageCommonRes<OceanStorageSystemInfoRes> getDeviceSystemInfo(URI url, @Param("iBaseToken") String iBaseToken,
        @Param("Cookie") String cookie);

    /**
     * 查询lun信息
     *
     * @param url url
     * @param iBaseToken 会话token
     * @param cookie cookie
     * @return luninfo
     */
    @ExterAttack
    @RequestLine("GET")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    StorageCommonRes<List<OceanStorageLunInfoRes>> getLunInfo(URI url, @Param("iBaseToken") String iBaseToken,
        @Param("Cookie") String cookie);

    /**
     * 获取免登陆的令牌
     *
     * @param url url
     * @param sessionReq sessionReq
     * @return 令牌信息
     */
    @ExterAttack
    @RequestLine("PUT")
    StorageCommonRes<OceanStorageSession> getToken(URI url, @RequestBody StorageSessionReq sessionReq);

    /**
     * 获取告警
     *
     * @param url url
     * @param iBaseToken iBaseToken
     * @param cookie cookie
     * @param lang lang
     * @return 告警列表
     */
    @ExterAttack
    @RequestLine("GET")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}", "Accept-Language:{Lang}"})
    StorageCommonRes<List<StorageAlarmResponse>> getLocalStorageAlarms(URI url, @Param("iBaseToken") String iBaseToken,
        @Param("Cookie") String cookie, @Param("Lang") String lang);

    /**
     * 获取告警总数量
     *
     * @param url url
     * @param iBaseToken iBaseToken
     * @param cookie cookie
     * @return 告警数量
     */
    @ExterAttack
    @RequestLine("GET")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    StorageCommonRes<StorageCountResponse> getLocalStorageAlarmCount(URI url, @Param("iBaseToken") String iBaseToken,
        @Param("Cookie") String cookie);

    /**
     * 通过ID获取你本地存储文件系统
     *
     * @param url url
     * @param iBaseToken iBaseToken
     * @param cookie cookie
     * @return 本地文件系统
     */
    @ExterAttack
    @RequestLine("GET")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    StorageCommonRes<StorageFileSystemBo> getStorageFileSystem(URI url, @Param("iBaseToken") String iBaseToken,
        @Param("Cookie") String cookie);

    /**
     * 通过ID查询文件系统双活pair信息
     *
     * @param url url
     * @param iBaseToken iBaseToken
     * @param cookie cookie
     * @return 本地文件系统
     */
    @ExterAttack
    @RequestLine("GET")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    StorageCommonRes<HyperMetroPairBo> getHyperMetroPair(URI url, @Param("iBaseToken") String iBaseToken,
        @Param("Cookie") String cookie);

    /**
     * 通过ID查询文件系统双活domain信息
     *
     * @param url url
     * @param iBaseToken iBaseToken
     * @param cookie cookie
     * @return 本地文件系统
     */
    @ExterAttack
    @RequestLine("GET")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    StorageCommonRes<HyperMetroDomainBo> getHyperMetroDomain(URI url, @Param("iBaseToken") String iBaseToken,
        @Param("Cookie") String cookie);

    /**
     * 通过ID获取你本地存储文件系统
     *
     * @param url url
     * @param iBaseToken iBaseToken
     * @param cookie cookie
     * @return 本地文件系统
     */
    @ExterAttack
    @RequestLine("GET")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    StorageCommonRes<StorageRemoteReplicationPairBo> getStorageRemoteReplicationPair(URI url,
        @Param("iBaseToken") String iBaseToken, @Param("Cookie") String cookie);

    /**
     * 清除告警
     *
     * @param url url
     * @param iBaseToken token
     * @param cookie cookie
     * @return 是否成功
     */
    @ExterAttack
    @RequestLine("DELETE")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    StorageCommonRes<Object> clearLocalStorageAlarms(URI url, @Param("iBaseToken") String iBaseToken,
        @Param("Cookie") String cookie);

    /**
     * 查询生产存储的逻辑端口信息
     *
     * @param url url
     * @param iBaseToken token
     * @param cookie cookie
     * @return 是否成功
     */
    @ExterAttack
    @RequestLine("GET")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    StorageCommonRes<List<OceanStorageLogincalPortRes>> queryLogicalPort(URI url,
        @Param("iBaseToken") String iBaseToken, @Param("Cookie") String cookie);

    /**
     * 查询单个文件系统快照信息
     *
     * @param url url
     * @param iBaseToken token
     * @param cookie cookie
     * @return 是否成功
     */
    @ExterAttack
    @RequestLine("GET")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    StorageCommonRes<StorageFileSystemSnapshotBo> getFsSnapshotDetail(URI url, @Param("iBaseToken") String iBaseToken,
        @Param("Cookie") String cookie);

    /**
     * 查询告警对象列表
     *
     * @param url url
     * @param iBaseToken token
     * @param cookie cookie
     * @return 告警对象列表
     */
    @ExterAttack
    @RequestLine("GET")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    StorageCommonRes<List<StorageAlarmObj>> getAlarmObjList(URI url, @Param("iBaseToken") String iBaseToken,
        @Param("Cookie") String cookie);
}