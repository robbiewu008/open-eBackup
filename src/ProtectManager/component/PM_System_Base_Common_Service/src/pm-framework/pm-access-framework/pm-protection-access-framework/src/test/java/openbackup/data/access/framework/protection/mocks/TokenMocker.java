package openbackup.data.access.framework.protection.mocks;

import openbackup.system.base.common.constants.TokenBo;

/**
 * token对象模拟
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/9/13
 **/
public class TokenMocker {
    /**
     * 获取模拟token对象
     *
     * @return token对象
     */
    public static TokenBo getMockedTokenBo() {
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setName("admin");
        userBo.setId("3434456567");
        long exp = System.currentTimeMillis();
        long created = System.currentTimeMillis();
        TokenBo tokenBo = TokenBo.builder().user(userBo).exp(exp).created(created).build();
        return tokenBo;
    }
}
