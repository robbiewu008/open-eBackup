package openbackup.system.base.sdk.cluster.model;

import lombok.Data;

import javax.validation.constraints.NotNull;

/**
 * TargetClusterInfoVo
 *
 * @author z00613137
 * @since 2023-05-25
 */
@Data
public class TargetClusterInfoVo {
    @NotNull
    private String token;

    @NotNull
    private TargetClusterInfo targetClusterInfo;
}