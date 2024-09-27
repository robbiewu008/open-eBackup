package openbackup.system.base.sdk.kmc;

import openbackup.system.base.sdk.kmc.model.CiphertextVo;
import openbackup.system.base.sdk.kmc.model.PlaintextVo;

/**
 * 解密api接口
 *
 * @author t00482481
 * @since 2020-07-05
 */
public interface EncryptorRestApi {
    /**
     * 解密
     *
     * @param ciphertextVo 解密对象
     * @return 解密后的产生的对象
     */
    PlaintextVo decrypt(CiphertextVo ciphertextVo);

    /**
     * 解密
     *
     * @param plaintextVo 加密密对象
     * @return 解密后的产生的对象
     */
    CiphertextVo encrypt(PlaintextVo plaintextVo);
}
