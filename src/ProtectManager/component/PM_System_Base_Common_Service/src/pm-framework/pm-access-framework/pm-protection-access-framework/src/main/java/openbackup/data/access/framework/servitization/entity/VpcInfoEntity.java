package openbackup.data.access.framework.servitization.entity;

import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Data;

/**
 * VpcInfoEntity
 *
 * @author l30044826
 * @since 2023-08-11
 */
@TableName(value = "t_vpc_info")
@Data
public class VpcInfoEntity {
    @TableId
    private String uuid;
    private String vpcId;
    private String projectId;
    private String markId;
}
