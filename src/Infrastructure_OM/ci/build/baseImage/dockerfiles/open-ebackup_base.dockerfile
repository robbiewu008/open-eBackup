#指定基础镜像
FROM openeuler-22.03-lts:latest

#jdk版本
ENV JDK_PACKAGE jdk_package
ENV JDK_VERSION jdk_version
ENV COMMON_INIT_VERSION product_version

WORKDIR /usr/local

#COPY openeuler.repo /etc/yum.repos.d/
COPY kmcv3.h /usr/include/
COPY sudoers /etc/
COPY --chown=99:99 kmcdecrypt /usr/bin/
COPY --chown=root:root restclient /usr/bin/
COPY --chown=99:99 get.sh /usr/bin/
COPY --chown=root:root init_logic_ports.py /usr/bin/
COPY --chown=root:root $JDK_VERSION /usr/local/$JDK_VERSION
#COPY --chown=99:99 libSecurityStarter.so /usr/lib64/
COPY --chown=99:99 FileClient /opt/FileClient
RUN cp /opt/FileClient/lib/libkmcv3.so /usr/lib64/  \
    && chmod 755 /usr/lib64/libkmcv3.so \
    && cp /opt/FileClient/lib/3rd/libcrypto.so.3 /usr/lib64/ \
    && chmod 755 /usr/lib64/libcrypto.so.3 \
    && cp /opt/FileClient/lib/libsecurec.so /usr/lib64/ \
    && chmod 755 /usr/lib64/libsecurec.so \
#    && chmod 550 /usr/lib64/libSecurityStarter.so \
    && echo "Defaults    env_keep += \"FILECLIENT_CONF_PATH FILECLIENT_LOG_PATH\"" >> /etc/sudoers
ADD common-init-${COMMON_INIT_VERSION}.tar.gz /usr/local/

RUN sed -i '/update/,$d' /etc/yum.repos.d/openEuler.repo
RUN dnf install -y python3-pip python39 openssl libatomic net-tools libuser shadow-utils openssh expect sudo iptables vim kmod libatomic dnsmasq nfs-utils\
    && chown -R root:root /usr/local/common-init \
    && dnf clean all \
    && chmod -R 555 "/usr/bin/restclient" \
    && ln -s /usr/bin/python3 /usr/bin/python

#环境配置
ENV JAVA_HOME /usr/local/$JDK_VERSION
ENV PATH $JAVA_HOME/bin:$PATH
RUN userdel nobody
RUN lgroupadd nobody -g 99

# 不需要登录的账号禁止登录，当前涉及root和dnsmasq两个账号
# 禁止root ssh登录
RUN sed -i "s/\/bin\/bash/\/sbin\/nologin/" /etc/passwd \
    && sed -i "s/PermitRootLogin yes/PermitRootLogin no/" /etc/ssh/sshd_config \

# 解決低权限目录/opt下面有root文件，导致提权
RUN rm -rf "/opt/euleros-base.json" "/opt/uvp" "/opt/oceanprotect-base.json"

# /usr/bin/newrole二进制权限太高，删除对应rpm包
RUN rpm -qa | grep ^policycoreutils-[0-9] | xargs -i rpm -e {} --nodeps

# 临时解决openeuler的漏洞, build阶段rpm没有权限, 升级时注意关注是否引入其他安全相关软件包，需要同步卸载
ENV tmp_openeuler_url "https://mirrors.tools.huawei.com/openeuler/openEuler-20.03-LTS-SP3/OS/aarch64/"
ENV base_openeuler_yum_url "https://mirrors.tools.huawei.com/openeuler/openEuler-20.03-LTS/everything/aarch64/"
RUN sed -i "/^baseurl=.*/c\baseurl=${tmp_openeuler_url}" /etc/yum.repos.d/openeuler.repo \
    && echo "y" | yum install augeas \
    && sed -i "/^baseurl=.*/c\baseurl=${base_openeuler_yum_url}" /etc/yum.repos.d/openeuler.repo