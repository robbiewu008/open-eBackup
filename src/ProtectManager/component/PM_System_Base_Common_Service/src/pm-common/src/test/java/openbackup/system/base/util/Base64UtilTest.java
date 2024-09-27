/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.util.Base64Util;

import org.junit.Assert;
import org.junit.Test;

import java.nio.file.Paths;

/**
 * The Base64Util 测试
 *
 * @author l50024885
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/22
 */
public class Base64UtilTest {
    /**
     * 用例场景：测试base64工具加密后是否能够回复到原内容
     * 前置条件：无
     * 检 查 点：加密之后解码能够恢复到相同内容
     */
    @Test
    public void encode_and_decode_context_success() {
        String context = "hello,world";
        String encryptedContext = Base64Util.encryptToBase64(context);
        String decryptedContext = Base64Util.decryptBase64ToString(encryptedContext);
        Assert.assertEquals(decryptedContext, context);

        String byteEncrypedContext = Base64Util.encryptToBase64(context.getBytes());
        String byteDecrypedContext = Base64Util.decryptBase64ToString(byteEncrypedContext);
        Assert.assertEquals(context, byteDecrypedContext);
    }

    /**
     * 用例场景：读取不存在的文件
     * 前置条件：无
     * 检 查 点：会报LegoCheckedException异常
     */
    @Test
    public void read_not_exist_file_failed() {
        Assert.assertThrows(LegoCheckedException.class, ()->{
            String encrypted = Base64Util.encryptToBase64(Paths.get("xx"));
        });
    }
}