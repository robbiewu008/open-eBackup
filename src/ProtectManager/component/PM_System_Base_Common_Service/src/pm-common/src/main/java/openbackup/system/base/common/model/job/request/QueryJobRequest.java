package openbackup.system.base.common.model.job.request;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import java.util.List;

import javax.validation.constraints.Min;
import javax.validation.constraints.Size;

/**
 * 查询任务请求
 *
 * @author w00448845
 * @version [BCManager 8.0.0]
 * @since 2020-04-15
 */
@Data
@JsonNaming(value = PropertyNamingStrategy.SnakeCaseStrategy.class)
public class QueryJobRequest {
    @Size(max = 32)
    private List<String> types;

    @JsonProperty("excludeTypes")
    private boolean isExcludeTypes;

    @Size(max = 1024)
    private List<String> sourceTypes;

    @Size(max = 16)
    private List<String> statusList;

    @Min(0)
    private Long fromStartTime;

    @Min(0)
    private Long toStartTime;

    @Size(max = 64)
    private String copyId;

    @Min(0)
    private Long fromCopyTime;

    @Min(0)
    private Long toCopyTime;

    @Min(0)
    private Long fromEndTime;

    @Min(0)
    private Long toEndTime;

    @Size(max = 256)
    private String sourceName;

    @Size(max = 1024)
    private String sourceLocation;

    @Size(max = 64)
    private String sourceId;

    @Size(max = 256)
    private String targetName;

    @Size(max = 1024)
    private String targetLocation;

    private Boolean isSystem;

    private Boolean isVisible;

    @Size(max = 64)
    private String jobId;

    /**
     * 扩展字段
     */
    @Size(max = 1024)
    private String extendStr;

    /**
     * 任务详情是否包含特定级别事件标志位
     */
    private Integer logLevels;

    @Size(max = 256)
    private String esn;

    /**
     * 节点名称
     */
    @Size(max = 256)
    private String clusterName;

    /**
     * 节点名称
     */
    @Size(max = 256)
    private String unitName;

    /**
     * slaId
     */
    @Size(max = 256)
    private List<String> slaIds;

    /**
     * 演练计划任务及其子任务存储演练id
     */
    @Size(max = 64)
    private String exerciseId;

    /**
     * 演练计划的子任务存储父任务id
     */
    @Size(max = 64)
    private String exerciseJobId;

    /**
     * 资源组id
     */
    @Size(max = 64)
    private String resourceGroupId;

    /**
     * 资源组备份任务id
     */
    @Size(max = 64)
    private String groupBackupJobId;

    /**
     * 任务处理状态： 0:不支持 1:未处理 2:已处理 3:已重试
     */
    @Size(max = 4)
    private List<@Size(max = 1) String> markStatus;

    /**
     * 是否用于恢复演练报表
     */
    private boolean isForExerciseReport;
}
