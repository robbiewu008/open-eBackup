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
package openbackup.data.access.framework.core.common.model;

/**
 * VMWare copy properties meta data
 *
 * @author h30003246
 * @since 2020/9/19
 **/
public final class VmIndexerCopyMetaData {
    /**
     * the VMWare disk info
     */
    public static final String DISK = "disk_info";

    /**
     * the VMWare net work info
     */
    public static final String NET_WORK = "net_work";

    /**
     * the VMWare contain hardware info
     */
    public static final String HARDWARE = "hardware";

    /**
     * the VMWare setting datastore;
     */
    public static final String VMX_DATASTORE = "vmx_datastore";

    /**
     * the VMWare runtime info
     */
    public static final String RUNTIME = "runtime";

    /**
     * the VMWare host info
     */
    public static final String HOST = "host";

    /**
     * the VMWare metadata
     */
    public static final String VMWARE_METADATA = "vmware_metadata";

    /**
     * certification key
     */
    public static final String CERTIFICATION = "certification";

    /**
     * revocation list key
     */
    public static final String REVOCATION_LIST = "revocation_list";

    /**
     * tls compatible key
     */
    public static final String TLS_COMPATIBLE = "tls_compatible";

    /**
     * 卷信息列表
     */
    public static final String VOL_LIST = "volList";

    /**
     * 快照信息
     */
    public static final String SNAPSHOTS = "snapshots";
}
