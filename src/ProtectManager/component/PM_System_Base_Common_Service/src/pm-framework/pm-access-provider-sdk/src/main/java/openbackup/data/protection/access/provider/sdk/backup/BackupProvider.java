/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.backup;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * This interface defines the protection providers that need to be implemented by DataMover, such as backup, archiving,
 * and copy copy.
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-19
 */
public interface BackupProvider extends DataProtectionProvider<String> {
    /**
     * The backup methods that need to be implemented by specific providers
     *
     * @param backupObject The parameters for backup
     */
    void backup(BackupObject backupObject);

    /**
     * remove protection
     *
     * @param protectedObject protected object
     */
    default void remove(ProtectedObject protectedObject) {
    }
}
