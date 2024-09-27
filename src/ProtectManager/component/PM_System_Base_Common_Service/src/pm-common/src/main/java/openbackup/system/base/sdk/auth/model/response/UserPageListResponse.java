package openbackup.system.base.sdk.auth.model.response;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;
import openbackup.system.base.common.model.PageListResponse;

import java.util.List;

/**
 * 分页模板类
 * 此类已经过时，新增代码不要引用。新增代码统一使用Common模块的分页对象
 * {@link PageListResponse}
 *
 * @param <T> the body type
 * @author w00448845
 * @version [BCManager 8.0.0]
 * @since 2019-11-12
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
@Deprecated
public class UserPageListResponse<T> {
    private int total;

    private Long currentCount;

    private Integer startIndex;

    private Integer pageSize;

    private List<T> userList;
}
