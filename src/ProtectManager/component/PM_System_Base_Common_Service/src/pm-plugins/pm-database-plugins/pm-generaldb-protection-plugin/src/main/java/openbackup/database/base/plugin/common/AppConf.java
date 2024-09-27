/*
 *
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
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
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-27
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
