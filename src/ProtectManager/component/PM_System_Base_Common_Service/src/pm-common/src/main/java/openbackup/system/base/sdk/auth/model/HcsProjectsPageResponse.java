package openbackup.system.base.sdk.auth.model;

import openbackup.system.base.sdk.auth.model.request.HcsUserProjectInfo;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 查询用户关联的资源空间列表返回体
 *
 * @author y30021475
 * @since 2023-07-27
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class HcsProjectsPageResponse {
    /**
     * 查询的用户关联资源空间的总数。
     */
    private int total;

    private List<HcsUserProjectInfo> projects;
}
