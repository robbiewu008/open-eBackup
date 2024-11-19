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
package openbackup.system.base.sdk.cloudbackup;

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;

import openbackup.system.base.common.constants.HyperMetroPair;
import openbackup.system.base.common.constants.LocalFileSystem;
import openbackup.system.base.common.constants.LocalRemoteReplicationPair;
import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;

/**
 * The CloudBackupApi
 *
 */
@FeignClient(name = "localFileSystemApi", url = "${service.url.pm-repository}/v1/internal/local-storage",
    configuration = CommonFeignConfiguration.class)
public interface LocalFileSystemApi {
    /**
     * 通过ID查找本地文件系统
     *
     * @param id ID
     * @return 本地文件系统
     */
    @ExterAttack
    @GetMapping("/filesystem/{id}")
    LocalFileSystem getLocalFileSystem(@PathVariable(name = "id") String id);

    /**
     * 通过ID查询本地远程复制Pair
     *
     * @param id ID
     * @return 本地远程复制Pair
     */
    @ExterAttack
    @GetMapping("/replicationpair/{id}")
    LocalRemoteReplicationPair getLocalRemoteReplicationPair(@PathVariable(name = "id") String id);

    /**
     * 通过ID查询双活Pair
     *
     * @param id ID
     * @return 文件系统双活域
     */
    @ExterAttack
    @GetMapping("/hypermetropair/{id}")
    HyperMetroPair getHyperMetroPair(@PathVariable(name = "id") String id);
}
