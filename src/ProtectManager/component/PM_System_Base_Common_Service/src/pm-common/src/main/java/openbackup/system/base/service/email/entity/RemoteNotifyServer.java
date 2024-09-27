package openbackup.system.base.service.email.entity;

import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 邮件服务器设置模型
 *
 * @author w00448845
 * @version @version [CDM Integrated machine]
 * @since 2019-10-28
 */
@NoArgsConstructor
@TableName(value = "t_alarm_notifyserver")
@Getter
@Setter
public class RemoteNotifyServer {
    // 主键
    @TableId("ID")
    private Long id;

    // 邮件服务器
    @TableField("SERVER")
    private String server;

    // 服务器端口（默认为25）
    @TableField("PORT")
    private int port;

    // 发件箱
    @TableField("EMAILFROM")
    private String emailFrom;

    @TableField("VALIDATEENABLE")
    private boolean isValidateEnable;

    // 用户名
    @TableField("USERNAME")
    private String userName;

    // 密码
    @TableField("PASSWORD")
    private String password;

    // 是否使用代理
    @TableField("PROXYENABLE")
    private boolean isProxyEnable;

    // 代理服务器
    @TableField("PROXYSERVER")
    private String proxyServer;

    // 端口（默认为1080）
    @TableField("PROXYPORT")
    private String proxyPort;

    // 邮件测试收件箱
    @TableField("TESTEMAIL")
    private String testEmail;

    // 当前邮件传输是否使用SSL加密
    @TableField("ISSSLENABLE")
    private Boolean isSslEnable;

    // 启用SSL SMTP服务器的端口号，默认端口（465）
    @TableField("SSLSMTPPORT")
    private Integer sslSmtpPort;

    // 当前邮件传输是否使用Tls加密
    @TableField("ISTLSENABLE")
    private Boolean isTlsEnable;
}
