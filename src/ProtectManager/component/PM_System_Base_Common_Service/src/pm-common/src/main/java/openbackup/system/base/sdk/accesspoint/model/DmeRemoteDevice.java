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
package openbackup.system.base.sdk.accesspoint.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * Dme Remote Device
 *
 * @author l00272247
 * @since 2020-12-16
 */
@Data
public class DmeRemoteDevice {
    private int port;

    @JsonProperty("portPM")
    private int portPm;

    @JsonProperty("userNamePM")
    private String userNamePm;

    @JsonProperty("passwordPM")
    private String passwordPm;

    @JsonProperty("tokenPM")
    private String tokenPM;

    private String cert;

    @JsonProperty("ESN")
    private String esn;

    @JsonProperty("mgrIp")
    private List<String> mgrIpList;

    @JsonProperty("netplaneinfo")
    private String netPlaneInfo;

    private String networkInfo;

    private String storageId;

    private String deployType;

    private String poolId;

    // op和存储不在一台机器上时，标识op的esn
    private String backupSoftwareEsn;

    // 远端存储类型
    private String remoteStorageType;
}
