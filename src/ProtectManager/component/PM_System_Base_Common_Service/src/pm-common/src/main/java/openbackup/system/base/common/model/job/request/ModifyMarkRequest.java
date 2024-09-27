package openbackup.system.base.common.model.job.request;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import javax.validation.constraints.Size;

/**
 * 修改处理意见请求
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-21
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class ModifyMarkRequest {
    @Size(max = 1024)
    private String mark;
}
