package openbackup.data.access.framework.core.model;

import com.fasterxml.jackson.annotation.JsonIgnore;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 副本资源查询条件
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-03-02
 */
@Getter
@Setter
public class CopySummaryResourceCondition {
    /**
     * 资源名称，模糊匹配
     */
    private String resourceName;

    /**
     * 资源位置，模糊匹配
     */
    private String resourceLocation;

    /**
     * 复制副本SLA名称，模糊匹配
     */
    private String protectedSlaName;

    /**
     * 资源子类型
     */
    private List<String> resourceSubType;

    /**
     * 资源状态
     */
    private List<String> resourceStatus;

    /**
     * 保护状态
     */
    private List<Boolean> protectedStatus;

    /**
     * 当前登录用户
     */
    @JsonIgnore
    private String userId;
}
