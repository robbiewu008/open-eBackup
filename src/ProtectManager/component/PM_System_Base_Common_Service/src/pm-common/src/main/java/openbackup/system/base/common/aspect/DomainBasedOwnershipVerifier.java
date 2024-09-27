package openbackup.system.base.common.aspect;

import openbackup.system.base.common.constants.TokenBo;

import java.util.List;

/**
 * resource domain based verifier
 *
 * @author l00272247
 * @since 2020-11-14
 */
public interface DomainBasedOwnershipVerifier {
    /**
     * verifier type
     *
     * @return verifier type
     */
    String getType();

    /**
     * verify
     *
     * @param userBo    user bo
     * @param resources resource uuid list
     */
    void verify(TokenBo.UserBo userBo, List<String> resources);
}
