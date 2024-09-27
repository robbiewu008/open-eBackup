package openbackup.system.base.sdk.livemount;

import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * Live Mount Ownership Verify Rest Api
 *
 * @author l00272247
 * @since 2020-11-28
 */
public interface LiveMountOwnershipVerifyRestApi {
    /**
     * verify resource ownership
     *
     * @param userId            user id
     * @param liveMountUuidList live mount uuid list
     */
    @ExterAttack
    @GetMapping("/live-mount/action/verify")
    @ResponseBody
    void verifyLiveMountOwnership(@RequestParam("user_id") String userId,
        @RequestParam("live_mount_uuid_list") List<String> liveMountUuidList);

    /**
     * verify resource ownership
     *
     * @param userId   user id
     * @param uuidList live mount policy uuid list
     */
    @ExterAttack
    @GetMapping("/live-mount-policy/action/verify")
    @ResponseBody
    void verifyLiveMountPolicyOwnership(@RequestParam("user_id") String userId,
        @RequestParam("policy_uuid_list") List<String> uuidList);
}
