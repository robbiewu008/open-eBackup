package openbackup.system.base.common.cmd;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * 恶意参数名单 华为C语言编程规范V5_1.pdf 附录B
 *
 * @since 2022-04-01
 */
public enum EvilParams {
    /**
     * 管道符
     */
    PIPE("|"),

    /**
     * 用于内联命令
     */
    INLINE(";"),

    /**
     * 用于后台命令
     */
    BACKGROUND("&"),

    /**
     * 用于命令引用
     */
    SUBSTITUTION("&"),

    /**
     * 重定向 >
     */
    REDIRECTION_OUT(">"),

    /**
     * 重定向 <
     */
    REDIRECTION_IN("<"),

    /**
     * 引用
     */
    QUOTE("`"),

    /**
     * 分隔符
     */
    BACK_SLASH("\\"),

    /**
     * 感叹号
     */
    EXCLAMATION("!"),

    /**
     * 回车
     */
    NEWLINE(System.lineSeparator());

    private final String param;

    EvilParams(String value) {
        this.param = value;
    }

    String getParam() {
        return param;
    }

    private static final Set<String> evilSets = new HashSet<>(
        Arrays.stream(EvilParams.values()).map(EvilParams::getParam).collect(Collectors.toList()));

    /**
     * 获取恶意参数黑名单
     *
     * @return 恶意参数黑名单集合
     */
    public static Set<String> getEvilParams() {
        return evilSets;
    }
}
