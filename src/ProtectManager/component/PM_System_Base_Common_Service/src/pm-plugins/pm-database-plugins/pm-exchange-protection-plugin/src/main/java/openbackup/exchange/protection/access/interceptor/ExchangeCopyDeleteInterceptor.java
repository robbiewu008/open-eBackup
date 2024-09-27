/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.exchange.protection.access.interceptor;

import openbackup.data.access.framework.copy.mng.provider.BaseCopyDeleteInterceptor;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/**
 * ExchangeCopyDeleteInterceptor
 *
 * @author z00830422
 * @since 2024/4/3 16:59
 */
@Slf4j
@Component
public class ExchangeCopyDeleteInterceptor extends BaseCopyDeleteInterceptor {
    private static final List<String> APPLICABLE_LIST = Arrays.asList(
            ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE.getType(),
            ResourceSubTypeEnum.EXCHANGE_GROUP.getType(),
            ResourceSubTypeEnum.EXCHANGE_DATABASE.getType()
    );

    @Override
    public boolean applicable(String object) {
        return APPLICABLE_LIST.contains(object);
    }

    /**
     * 获取永久增量副本级联关系
     * <p>
     * 日志副本和永久增量副本有级联关系，需要一并删除
     * <p>
     * 获取 [当前增量副本-下一个全量副本或永久增量副本) 中类型为"日志"的副本
     * 例如（增量代表永久增量）:
     * 如[1日志、2日志、3全量、4日志]，会过滤出[1日志、2日志]
     * 如[1日志、2日志、3日志、4增量]，会过滤出[1日志、2日志、3日志]
     * 如[1全量、2日志、3日志、4日志]，会过滤出[]
     * 如[1增量、2日志、3日志、4日志]，会过滤出[]
     * 如[1日志、2日志、3日志、4日志]，会过滤出[1日志、2日志、3日志、4日志]
     *
     * @param copies       本个副本之后的所有备份副本
     * @param thisCopy     本个副本
     * @param nextFullCopy 下个全量副本
     * @return 待删除列表
     */
    @Override
    protected List<String> getCopiesCopyTypeIsPermanentIncrement(
            List<Copy> copies, Copy thisCopy, Copy nextFullCopy
    ) {
        // 找到本个副本之后第一次出现的永久增量或全量
        Copy firstPermanentIncrement = copies.stream()
                .filter(copy -> {
                    BackupTypeConstants copyBackupType = BackupTypeConstants.getBackupTypeByAbBackupType(
                            copy.getBackupType()
                    );
                    return BackupTypeConstants.PERMANENT_INCREMENT.equals(copyBackupType)
                            || BackupTypeConstants.FULL.equals(copyBackupType);
                })
                .findFirst()
                .orElse(null);

        // 只取范围内的日志副本
        List<Copy> logCopies = copies.stream()
                .filter(copy -> BackupTypeConstants.LOG.equals(
                        BackupTypeConstants.getBackupTypeByAbBackupType(copy.getBackupType())
                ))
                .collect(Collectors.toList());

        // 通过副本代数 找到(本个副本 - 本个副本之后第一次出现的永久增量或全量)之间的全部日志副本
        return CopyUtil.getCopyUuidsBetweenTwoCopy(logCopies, thisCopy, firstPermanentIncrement);
    }
}