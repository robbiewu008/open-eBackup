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
package openbackup.data.access.framework.copy.mng.util;

import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;

import org.apache.commons.lang3.StringUtils;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Optional;
import java.util.OptionalInt;
import java.util.stream.Collectors;

import javax.validation.constraints.NotNull;

/**
 * CopyUtil
 *
 */
public class CopyUtil {
    /**
     * 获取gn以后的下个指定类型的副本
     *
     * @param copies 副本集合
     * @param copyType 副本类型
     * @param gn gn
     * @return Copy
     */
    public static Copy getNextCopyByType(List<Copy> copies, BackupTypeConstants copyType, int gn) {
        return copies.stream()
            .filter(copy -> copy.getGn() > gn && match(copyType, copy.getBackupType()))
            .min(Comparator.comparingInt(Copy::getGn))
            .orElse(null);
    }

    /**
     * 获取gn以后的下个全量副本
     *
     * @param copies 副本集合
     * @param gn gn
     * @return Copy
     */
    public static Copy getNextFullCopy(List<Copy> copies, int gn) {
        return getNextCopyByType(copies, BackupTypeConstants.FULL, gn);
    }

    /**
     * 获取gn以后的下个增量副本
     *
     * @param copies 副本集合
     * @param gn gn
     * @return Copy
     */
    public static Copy getNextDifferenceCopy(List<Copy> copies, int gn) {
        return getNextCopyByType(copies, BackupTypeConstants.DIFFERENCE_INCREMENT, gn);
    }

    /**
     * 获取指定类型的副本
     *
     * @param copies 副本集合
     * @param copyType 副本类型
     * @return List<Copy>
     */
    public static List<Copy> getCopiesByCopyType(List<Copy> copies, BackupTypeConstants copyType) {
        return copies.stream().filter(copy -> match(copyType, copy.getBackupType())).collect(Collectors.toList());
    }

    /**
     * 获取两个之间的副本uuid集合
     *
     * @param copies 副本集合
     * @param nextCopy 下个副本
     * @param thisCopy 本副本
     * @return Copy uuids
     */
    public static List<String> getCopyUuidsBetweenTwoCopy(List<Copy> copies, @NotNull Copy thisCopy, Copy nextCopy) {
        return getCopiesBetweenTwoCopy(copies, thisCopy, nextCopy).stream().map(Copy::getUuid)
            .collect(Collectors.toList());
    }

    /**
     * 获取两个副本之间的副本列表
     *
     * @param copies 副本集合
     * @param thisCopy 本副本
     * @param nextCopy 下个副本
     * @return Copy列表
     */
    public static List<Copy> getCopiesBetweenTwoCopy(List<Copy> copies, @NotNull Copy thisCopy, Copy nextCopy) {
        return copies.stream().filter(copy -> {
            if (nextCopy == null) {
                return copy.getGn() > thisCopy.getGn();
            }
            return copy.getGn() > thisCopy.getGn() && copy.getGn() < nextCopy.getGn();
        }).collect(Collectors.toList());
    }

    /**
     * 匹配副本类型
     *
     * @param copyType 副本类型枚举
     * @param type 副本类型值
     * @return boolean
     */
    public static boolean match(BackupTypeConstants copyType, int type) {
        return type == copyType.getAbBackupType();
    }

    /**
     * 匹配副本生成类型
     *
     * @param generatedBy 副本生成类型枚举
     * @param generated 副本生成类型值
     * @return boolean
     */
    public static boolean match(CopyGeneratedByEnum generatedBy, String generated) {
        return generatedBy.value().equals(generated);
    }

    /**
     * 匹配副本生成类型
     *
     * @param generatedByEnumList 副本生成类型枚举list
     * @param generated 副本生成类型值
     * @return boolean
     */
    public static boolean match(List<CopyGeneratedByEnum> generatedByEnumList, String generated) {
        if (VerifyUtil.isEmpty(generatedByEnumList)) {
            return false;
        }
        return generatedByEnumList.stream().anyMatch(generatedBy -> generatedBy.value().equals(generated));
    }

    /**
     * getSmallerCopy
     *
     * @param firstCopy firstCopy
     * @param secondCopy secondCopy
     * @return Copy
     */
    public static Copy getSmallerCopy(Copy firstCopy, Copy secondCopy) {
        if (firstCopy == null) {
            return secondCopy;
        }
        if (secondCopy == null) {
            return firstCopy;
        }
        return firstCopy.getGn() < secondCopy.getGn() ? firstCopy : secondCopy;
    }

    /**
     * 获取紧跟差异副本后的所有日志副本
     *
     * @param copies 副本
     * @param thisCopy 原副本
     * @param nextFullCopy 下个全量副本
     * @return 所有日志副本
     */
    public static List<String> getLogCopiesBeforeNextNonLogCopy(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        int nextFullCopyGn = nextFullCopy == null ? Integer.MAX_VALUE : nextFullCopy.getGn();
        List<String> logCopies = new ArrayList<>();
        int nextGn = thisCopy.getNextCopyGn() == 0 ? thisCopy.getGn() + 1 : thisCopy.getNextCopyGn();

        while (nextGn > thisCopy.getGn() && nextGn < nextFullCopyGn) {
            int finalNextGn = nextGn;
            Optional<Copy> nextCopy = copies.stream()
                .filter(copy -> copy.getGn() == finalNextGn
                    && BackupTypeConstants.LOG.getAbBackupType() == copy.getBackupType())
                .findFirst();

            if (nextCopy.isPresent()) {
                logCopies.add(nextCopy.get().getUuid());
                nextGn = nextCopy.get().getNextCopyGn();
            } else {
                return logCopies;
            }
        }
        return logCopies;
    }

    /**
     * 判断副本是否通过备份生成
     *
     * @param copyRestApi 副本接口
     * @param copy 副本
     * @return boolean
     */
    public static boolean isGeneratedByBackup(CopyRestApi copyRestApi, CopyInfoBo copy) {
        Copy thisCopy = copyRestApi.queryCopyByID(copy.getUuid());
        return match(CopyGeneratedByEnum.BY_BACKUP, thisCopy.getGeneratedBy());
    }

    /**
     * 根据副本properties获取副本格式
     *
     * @param copy 副本
     * @return 副本格式
     */
    public static OptionalInt getFormat(Copy copy) {
        String properties = copy.getProperties();
        if (VerifyUtil.isEmpty(properties)) {
            return OptionalInt.empty();
        }
        JSONObject jsonObject = JSONObject.fromObject(properties);
        Object format = jsonObject.get("format");
        if (format == null) {
            return OptionalInt.empty();
        }
        return OptionalInt.of(Integer.parseInt(format.toString()));
    }

    /**
     * 根据副本properties获取副本校验状态
     *
     * @param copy 副本
     * @return 副本校验状态
     */
    public static String getVerifyStatus(Copy copy) {
        String properties = copy.getProperties();
        if (VerifyUtil.isEmpty(properties)) {
            return StringUtils.EMPTY;
        }
        JSONObject jsonObject = JSONObject.fromObject(properties);
        Object verifyStatus = jsonObject.get(CopyPropertiesKeyConstant.KEY_VERIFY_STATUS);
        if (verifyStatus == null) {
            return StringUtils.EMPTY;
        }
        return verifyStatus.toString();
    }

    /**
     * 根据副本properties中的san标志判断是否是san副本
     *
     * @param copy 副本
     * @return 是否是san副本
     */
    public static Boolean checkIsSanCopy(Copy copy) {
        String properties = copy.getProperties();
        if (VerifyUtil.isEmpty(properties)) {
            return Boolean.FALSE;
        }
        JSONObject jsonObject = JSONObject.fromObject(properties);
        Object isSanClient = jsonObject.get("isSanClient");
        if (!VerifyUtil.isEmpty(isSanClient) && Boolean.TRUE.toString().equals(isSanClient.toString())) {
            return Boolean.TRUE;
        }
        return Boolean.FALSE;
    }
}
