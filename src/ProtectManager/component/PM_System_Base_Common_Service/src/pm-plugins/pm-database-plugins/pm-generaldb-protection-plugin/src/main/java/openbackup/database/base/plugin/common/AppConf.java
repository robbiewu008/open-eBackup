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
package openbackup.database.base.plugin.common;

import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 应用配置conf
 *
 */
@Setter
@Getter
public class AppConf {
    private String databaseType;

    private Resource resource;

    private Backup backup;

    private Restore restore;

    private Copy copy;

    /**
     * 应用配置-资源配置
     */
    @Getter
    @Setter
    public static class Resource {
        private Double clusterCheckResultThreshold = 1d;
        private String customParamsTemplate;
    }

    /**
     * 应用配置-备份配置
     */
    @Setter
    @Getter
    public static class Backup {
        private String speedStatistics = SpeedStatisticsEnum.UBC.getType();

        /**
         * 是否多节点执行
         */
        @JsonProperty("multiPostJob")
        private Boolean isMultiPostJob = false;

        @JsonProperty("support")
        private List<Support> supports;

        /**
         * 日志备份是否需要DATA仓
         */
        @JsonProperty("logBackupNeedDataRepository")
        private boolean isLogBackupNeedDataRepository = false;

        /**
         * 应用配置-备份配置-支持的备份
         */
        @Getter
        @Setter
        public static class Support {
            private String backupType;

            private String minVersion;

            private String maxVersion;
        }
    }

    /**
     * 应用配置-恢复配置
     */
    @Setter
    @Getter
    public static class Restore {
        private String speedStatistics = SpeedStatisticsEnum.UBC.getType();

        /**
         * 是否多节点执行
         */
        @JsonProperty("multiPostJob")
        private Boolean isMultiPostJob = false;

        @JsonProperty("support")
        private List<Support> supports;

        /**
         * 应用配置-恢复配置-支持的恢复
         */
        @Getter
        @Setter
        public static class Support {
            private String restoreType;

            @JsonProperty("includeBackupType")
            private List<String> includeBackupTypes;

            private String minVersion;

            private String maxVersion;
        }
    }

    /**
     * 应用配置-副本配置
     */
    @Setter
    @Getter
    public static class Copy {
        private Integer format = CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat();

        @JsonProperty("delete")
        private List<Delete> deletes;

        /**
         * 应用配置-副本配置-删除的配置
         */
        @Getter
        @Setter
        public static class Delete {
            @JsonProperty("deleteWithAgent")
            private Boolean isDeleteWithAgent = false;

            private List<String> backupType;

            private List<String> associatedType;
        }
    }
}
