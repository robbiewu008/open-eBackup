package openbackup.system.base.sdk.job.model.request;

import lombok.Getter;
import lombok.Setter;

import java.util.Map;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.Size;

/**
 * 高级配置，k8s恢复才有
 *
 * @author l30057246
 * @since 2024-05-11
 */
@Setter
@Getter
public class ScParameter {
    /**
     * 储存类名字
     */
    @NotEmpty
    @Size(min = 1, max = 50)
    private String scName;

    /**
     * 参数
     */
    @NotEmpty
    @Size(min = 1, max = 10)
    private Map<String, String> paramMap;
}
