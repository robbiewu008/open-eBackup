/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
