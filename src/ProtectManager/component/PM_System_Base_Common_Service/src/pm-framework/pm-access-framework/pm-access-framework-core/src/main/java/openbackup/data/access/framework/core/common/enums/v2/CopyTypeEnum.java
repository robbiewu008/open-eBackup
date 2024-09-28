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
package openbackup.data.access.framework.core.common.enums.v2;

import java.util.Arrays;

/**
 * 副本类型定义，定义DME副本类型和PM的副本类型映射
 *
 */
public enum CopyTypeEnum {
    // 副本类型：
    // nativeSnapshot：本地快照 | full：全量 | increment：增量 | diff：差异增量
    // | foreverIncrement：永久增量 | log：日志 | replication：复制 | s3Archive：S3归档
    // | tapeArchive：磁带归档 | clone：克隆副本 存入md；
    /**
     * 原生快照副本,快照直接存储生产存储上，如同构NAS备份，直接对生产存储打快照
     */
    NATIVE_SNAPSHOT("nativeSnapshot", "snapshot", "快照", "Snapshot"),

    /**
     * 全量备份副本
     */
    FULL_BACKUP_COPY("full", "full", "全量", "Full"),

    /**
     * 增量备份副本
     */
    INCREMENT_BACKUP_COPY("increment", "difference_increment", "增量", "Incremental"),
    /**
     * 永久增量备份副本
     */
    FOREVER_INCREMENT_BACKUP_COPY("foreverIncrement", "permanent_increment", "永久增量（合成全量）",
        "Permanent Incremental (Synthetic Full)"),

    /**
     * 差异增量备份副本
     */
    DIFF_BACKUP_COPY("diff", "cumulative_increment", "差异", "Differential"),

    /**
     * 日志备份副本
     */
    LOG_BACKUP_COPY("log", "log", "日志", "Log"),

    /**
     * S3归档副本
     */
    S3_ARCHIVE_COPY("s3Archive", "s3Archive", "对象存储归档", "Object Storage Archive"),

    /**
     * 复制副本
     */
    REPLICATION_COPY("replication", "replication", "复制", "Replication"),

    /**
     * 磁带归档副本
     */
    TAPE_ARCHIVE_COPY("tapeArchive", "tapeArchive", "磁带归档", "Tape Archive");

    // DME上定义的副本类型
    private String dmeCopyType;

    // PM上定义的副本类型
    private String copyType;

    private final String nameCn;

    private final String nameEn;

    public String getDmeCopyType() {
        return dmeCopyType;
    }

    public String getCopyType() {
        return copyType;
    }

    /**
     * get nameCn
     *
     * @return string
     */
    public String getNameCn() {
        return nameCn;
    }

    /**
     * get nameEn
     *
     * @return string
     */
    public String getNameEn() {
        return nameEn;
    }

    CopyTypeEnum(String dmeCopyType, String copyType, String nameCn, String nameEn) {
        this.dmeCopyType = dmeCopyType;
        this.copyType = copyType;
        this.nameCn = nameCn;
        this.nameEn = nameEn;
    }

    /**
     * 根据DME的副本类型获取管控面统一的副本类型
     *
     * @param dmeCopyType DME上的副本类型
     * @return 返回管控面统一的副本类型
     */
    public static CopyTypeEnum getCopyType(String dmeCopyType) {
        return Arrays.stream(CopyTypeEnum.values())
                .filter(type -> type.dmeCopyType.equals(dmeCopyType))
                .findFirst()
                .orElseThrow(IllegalArgumentException::new);
    }
}
