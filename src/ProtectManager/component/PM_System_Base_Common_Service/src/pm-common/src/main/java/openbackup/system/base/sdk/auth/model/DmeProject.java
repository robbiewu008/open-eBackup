package openbackup.system.base.sdk.auth.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * Dme资源集对象属性
 *
 * @author z30062305
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-29
 */
@Setter
@Getter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class DmeProject {
    /**
     * 资源集ID
     */
    private String id;

    /**
     * 资源集名称
     */
    private String name;

    /**
     * 资源集描述
     */
    private String description;

    /**
     * 资源集使能开关
     */
    private boolean enabled;

    /**
     * domain id
     */
    private String domainId;

    /**
     * domain名称
     */
    private String domainName;

    /**
     * VDC id
     */
    private String vdcId;

    /**
     * VDC名称
     */
    private String vdcName;

    /**
     * 创建时间
     */
    private Long createTime;

    /**
     * 资源集关联地域
     */
    private List<DmeProjectRegion> regions;
}