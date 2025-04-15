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
package openbackup.exchange.protection.access.interceptor;

import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.Copy;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.junit.jupiter.MockitoExtension;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;

/**
 * ExchangeCopyDeleteInterceptorTest
 *
 */
@ExtendWith(MockitoExtension.class)
public class ExchangeCopyDeleteInterceptorTest {
    @Test
    @DisplayName("测试永久增量后是日志和全量")
    void test_log_and_full_are_generated_after_permanent_increments() {
        // Arrange
        Copy thisCopy = newCopy(BackupTypeConstants.PERMANENT_INCREMENT, 1);
        List<Copy> copies = Arrays.asList(
                newCopy(BackupTypeConstants.LOG, 2),
                newCopy(BackupTypeConstants.LOG, 3),
                newCopy(BackupTypeConstants.FULL, 4),
                newCopy(BackupTypeConstants.LOG, 5)
        );
        Copy nextFullCopy = copies.get(2);

        // Action
        List<String> res = new ExchangeCopyDeleteInterceptor()
                .getCopiesCopyTypeIsPermanentIncrement(copies, thisCopy, nextFullCopy);

        // Assert
        Assertions.assertEquals(
                copies.subList(0, 2).stream().map(Copy::getUuid).collect(Collectors.toList()),
                res
        );
    }

    @Test
    @DisplayName("测试永久增量后是日志和永久增量")
    void test_log_and_permanent_increment_are_generated_after_permanent_increments() {
        // Arrange
        Copy thisCopy = newCopy(BackupTypeConstants.PERMANENT_INCREMENT, 1);
        List<Copy> copies = Arrays.asList(
                newCopy(BackupTypeConstants.LOG, 2),
                newCopy(BackupTypeConstants.LOG, 3),
                newCopy(BackupTypeConstants.LOG, 4),
                newCopy(BackupTypeConstants.PERMANENT_INCREMENT, 5)
        );
        Copy nextFullCopy = null;

        // Action
        List<String> res = new ExchangeCopyDeleteInterceptor()
                .getCopiesCopyTypeIsPermanentIncrement(copies, thisCopy, nextFullCopy);

        // Assert
        Assertions.assertEquals(
                copies.subList(0, 3).stream().map(Copy::getUuid).collect(Collectors.toList()),
                res
        );
    }

    @Test
    @DisplayName("测试永久增量后是全量")
    void test_full_are_generated_after_permanent_increments() {
        // Arrange
        Copy thisCopy = newCopy(BackupTypeConstants.PERMANENT_INCREMENT, 1);
        List<Copy> copies = Arrays.asList(
                newCopy(BackupTypeConstants.FULL, 2),
                newCopy(BackupTypeConstants.LOG, 3),
                newCopy(BackupTypeConstants.LOG, 4),
                newCopy(BackupTypeConstants.LOG, 5)
        );
        Copy nextFullCopy = copies.get(0);

        // Action
        List<String> res = new ExchangeCopyDeleteInterceptor()
                .getCopiesCopyTypeIsPermanentIncrement(copies, thisCopy, nextFullCopy);

        // Assert
        Assertions.assertEquals(
                Collections.emptyList(),
                res
        );
    }

    @Test
    @DisplayName("测试永久增量后是永久增量")
    void test_permanent_increment_are_generated_after_permanent_increments() {
        // Arrange
        Copy thisCopy = newCopy(BackupTypeConstants.PERMANENT_INCREMENT, 1);
        List<Copy> copies = Arrays.asList(
                newCopy(BackupTypeConstants.PERMANENT_INCREMENT, 2),
                newCopy(BackupTypeConstants.LOG, 3),
                newCopy(BackupTypeConstants.LOG, 4),
                newCopy(BackupTypeConstants.LOG, 5)
        );
        Copy nextFullCopy = null;

        // Action
        List<String> res = new ExchangeCopyDeleteInterceptor()
                .getCopiesCopyTypeIsPermanentIncrement(copies, thisCopy, nextFullCopy);

        // Assert
        Assertions.assertEquals(
                Collections.emptyList(),
                res
        );
    }

    @Test
    @DisplayName("测试永久增量后没有全量或永久增量")
    void test_no_full_are_generated_after_permanent_increments() {
        // Arrange
        Copy thisCopy = newCopy(BackupTypeConstants.PERMANENT_INCREMENT, 1);
        List<Copy> copies = Arrays.asList(
                newCopy(BackupTypeConstants.LOG, 2),
                newCopy(BackupTypeConstants.LOG, 3),
                newCopy(BackupTypeConstants.LOG, 4),
                newCopy(BackupTypeConstants.LOG, 5)
        );
        Copy nextFullCopy = null;

        // Action
        List<String> res = new ExchangeCopyDeleteInterceptor()
                .getCopiesCopyTypeIsPermanentIncrement(copies, thisCopy, nextFullCopy);

        // Assert
        Assertions.assertEquals(
                copies.stream().map(Copy::getUuid).collect(Collectors.toList()),
                res
        );
    }

    private Copy newCopy(BackupTypeConstants backupType, int gn) {
        Copy copy = new Copy();
        JSONObject jsonObject = new JSONObject();
        jsonObject.put("format", CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        copy.setProperties(jsonObject.toString());
        copy.setBackupType(backupType.getAbBackupType());
        copy.setGn(gn);
        copy.setUuid("uuid_" + gn);
        return copy;
    }
}