package openbackup.system.base.common.utils.security;

import openbackup.system.base.sdk.auth.api.KeyManagerService;
import openbackup.system.base.sdk.kmc.EncryptorRestApi;
import openbackup.system.base.sdk.kmc.model.CiphertextVo;
import openbackup.system.base.sdk.kmc.model.PlaintextVo;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.concurrent.atomic.AtomicReference;

/**
 * 加解密
 *
 * @author p00511147
 * @version [CDM Integrated machine]
 * @since 2020-12-09
 */
@Component
public class EncryptorUtil implements KeyManagerService {
    private static final AtomicReference<EncryptorUtil> INSTANCE = new AtomicReference<>();

    @Autowired
    private EncryptorRestApi encryptorRestApi;

    /**
     * 无参构造器
     */
    public EncryptorUtil() {
        final EncryptorUtil previous = INSTANCE.getAndSet(this);
        if (previous != null) {
            throw new IllegalStateException("Second singleton " + this + " created after " + previous);
        }
    }

    public static EncryptorUtil getInstance() {
        return INSTANCE.get();
    }

    /**
     * 加密
     *
     * @param plaintext 明文
     * @return 密文
     */
    @Override
    public String getEncryptPwd(String plaintext) {
        PlaintextVo plaintextVo = new PlaintextVo();
        plaintextVo.setPlaintext(plaintext);
        return encryptorRestApi.encrypt(plaintextVo).getCiphertext();
    }

    /**
     * 解密
     *
     * @param decryptPwd 密文
     * @return 明文
     */
    @Override
    public String getDecryptPwd(String decryptPwd) {
        CiphertextVo ciphertextVo = new CiphertextVo();
        ciphertextVo.setCiphertext(decryptPwd);
        return encryptorRestApi.decrypt(ciphertextVo).getPlaintext();
    }
}
