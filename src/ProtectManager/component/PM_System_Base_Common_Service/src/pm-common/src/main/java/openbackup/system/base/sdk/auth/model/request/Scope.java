package openbackup.system.base.sdk.auth.model.request;

import openbackup.system.base.sdk.auth.model.Domain;

import lombok.Getter;
import lombok.Setter;

/**
 * HcsTokenAuth 字段
 * 取值为domain时，表示获取的Token可以访问指定账号下所有资源，domain支持id和name
 *
 * @author l30044826
 * @since 2023-07-07
 */
@Getter
@Setter
public class Scope {
    private Domain domain;

    private Project project;
}
