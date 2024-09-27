package openbackup.access.framework.resource.persistence.model;

import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Getter;
import lombok.Setter;

/**
 * 资源脱敏对象
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-04-08
 */
@Getter
@Setter
@TableName(value = "resource_desesitization")
public class ResourceDesesitizationPo {
    @TableId
    private String uuid;

    /**
     * 脱敏状态
     */
    private String desesitizationStatus;

    /**
     * 识别状态
     */
    private String identificationStatus;

    /**
     * 脱敏任务id
     */
    private String desesitizationJobId;

    /**
     * 识别任务id
     */
    private String identificationJobId;

    /**
     * 脱敏策略id
     */
    private String desesitizationPolicyId;

    /**
     * 脱敏策略名称
     */
    private String desesitizationPolicyName;
}
