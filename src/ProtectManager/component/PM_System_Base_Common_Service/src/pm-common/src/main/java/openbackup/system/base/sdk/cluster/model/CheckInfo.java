package openbackup.system.base.sdk.cluster.model;

import lombok.Getter;
import lombok.NonNull;
import lombok.Setter;

/**
 * 功能说明 检查满足度结果
 *
 * @author x30046484
 * @since 2023-05-11
 */

@Getter
@Setter
public class CheckInfo {
    /**
     * 检查结果
     */
    @NonNull
    private Integer checkResult;

    /**
     * 失败类型
     */
    private Integer failureType;
}
