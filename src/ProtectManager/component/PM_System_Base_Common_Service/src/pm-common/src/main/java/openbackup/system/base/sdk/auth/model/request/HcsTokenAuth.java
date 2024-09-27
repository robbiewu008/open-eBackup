package openbackup.system.base.sdk.auth.model.request;

import lombok.Getter;
import lombok.Setter;

/**
 * HSC token 认证字段
 *
 * @author l30044826
 * @since 2023-07-07
 */
@Getter
@Setter
public class HcsTokenAuth {
    private Identity identity;

    private Scope scope;
}
