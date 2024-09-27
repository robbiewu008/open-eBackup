package openbackup.system.base.common.aspect.verifier;

import openbackup.system.base.common.aspect.DomainBasedOwnershipVerifier;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.utils.VerifyUtil;

import java.util.List;
import java.util.function.BiConsumer;

/**
 * Common Ownership Verifier
 *
 * @author l00272247
 * @since 2020-11-28
 */
public class CommonOwnershipVerifier implements DomainBasedOwnershipVerifier {
    private final String type;

    private final BiConsumer<String, List<String>> verifier;

    /**
     * constructor
     *
     * @param type     type
     * @param verifier verifier
     */
    public CommonOwnershipVerifier(String type, BiConsumer<String, List<String>> verifier) {
        this.type = type;
        this.verifier = verifier;
    }

    /**
     * verifier type
     *
     * @return verifier type
     */
    @Override
    public String getType() {
        return type;
    }

    /**
     * verify
     *
     * @param userBo    user bo
     * @param resources resource uuid list
     */
    @Override
    public void verify(TokenBo.UserBo userBo, List<String> resources) {
        if (VerifyUtil.isEmpty(resources)) {
            return;
        }
        verifier.accept(userBo.getId(), resources);
    }
}
