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
package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import openbackup.system.base.sdk.storage.enums.DoradoRunningStatus;

/**
 * dorado设备信息
 *
 */
@Data
public class OceanStorageSystemInfoRes {
    @JsonProperty("CACHEWRITEQUOTA")
    private String cacheWriteQuota;

    @JsonProperty("CONFIGMODEL")
    private String configModel;

    @JsonProperty("DESCRIPTION")
    private String description;

    @JsonProperty("DOMAINNAME")
    private String domainName;

    @JsonProperty("FREEDISKSCAPACITY")
    private String freeDisksCapacity;

    @JsonProperty("HEALTHSTATUS")
    private String healthStatus;

    @JsonProperty("HOTSPAREDISKSCAPACITY")
    private String hotSpareDisksCapaCity;

    @JsonProperty("ID") // 设备序列号
    private String esn;

    @JsonProperty("LOCATION")
    private String location;

    @JsonProperty("MEMBERDISKSCAPACITY")
    private String memberDisksCapacity;

    @JsonProperty("NAME")
    private String deviceName;

    @JsonProperty("PRODUCTMODE")
    private String productMode;

    @JsonProperty("PRODUCTVERSION")
    private String productVersion;

    @JsonProperty("RUNNINGSTATUS")
    private DoradoRunningStatus runningStatus;

    @JsonProperty("SECTORSIZE")
    private String sectorSize;

    @JsonProperty("STORAGEPOOLCAPACITY")
    private String storagePoolCapacity;

    @JsonProperty("STORAGEPOOLFREECAPACITY")
    private String storagePool;

    @JsonProperty("STORAGEPOOLHOSTSPARECAPACITY")
    private String storagePoolHostSpareCapacity;

    @JsonProperty("STORAGEPOOLRAWCAPACITY")
    private String storagePoolRawCapacity;

    @JsonProperty("STORAGEPOOLUSEDCAPACITY")
    private String storagePoolUsedCapacity;

    @JsonProperty("THICKLUNSALLOCATECAPACITY")
    private String thickLunSallocateCapacity;

    @JsonProperty("THICKLUNSUSEDCAPACITY")
    private String thickLunsUsedCapacity;

    @JsonProperty("THINLUNSALLOCATECAPACITY")
    private String thinLunSallocateCapacity;

    @JsonProperty("THINLUNSMAXCAPACITY")
    private String thinLunSmaxCapacity;

    @JsonProperty("THINLUNSUSEDCAPACITY")
    private String thinLunsUsedCapacity;

    @JsonProperty("TYPE")
    private String type;

    @JsonProperty("UNAVAILABLEDISKSCAPACITY")
    private String unavailableDisksCapacity;

    @JsonProperty("USEDCAPACITY")
    private String usedCapacity;

    @JsonProperty("VASA_ALTERNATE_NAME")
    private String vasaAltrnateName;

    @JsonProperty("VASA_SUPPORT_BLOCK")
    private String vasaSupportBlock;

    @JsonProperty("VASA_SUPPORT_FILESYSTEM")
    private String vasaSupportFileSystem;

    @JsonProperty("VASA_SUPPORT_PROFILE")
    private String vasaSupportProfile;

    @JsonProperty("WRITETHROUGHSW")
    private String writeThroughSW;

    @JsonProperty("WRITETHROUGHTIME")
    private String wirteThroughTime;

    @JsonProperty("mappedLunsCountCapacity")
    private String mappedLunsCountCapacity;

    @JsonProperty("patchVersion")
    private String pathVersion;

    @JsonProperty("unMappedLunsCountCapacity")
    private String unMappedLunsCountCapacity;

    @JsonProperty("userFreeCapacity")
    private String userFreeCapacity;

    @JsonProperty("wwn")
    private String wwn;

    @JsonProperty("pointRelease")
    private String version;

    @JsonProperty("productModeString")
    private String productModeString;

    @JsonProperty("TOTALCAPACITY")
    private String totalCapacity;
}
