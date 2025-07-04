#指定基础镜像
FROM openeuler-22.03-lts:latest

#jdk版本
ENV JDK_PACKAGE jdk_package
ENV JDK_VERSION jdk_version
ENV COMMON_INIT_VERSION product_version

WORKDIR /usr/local

# COPY openeuler.repo /etc/yum.repos.d/
COPY kmcv3.h /usr/include/
COPY sudoers /etc/
COPY --chown=99:99 kmcdecrypt /usr/bin/
COPY --chown=root:root restclient /usr/bin/
COPY --chown=99:99 get.sh /usr/bin/
COPY --chown=root:root init_logic_ports.py /usr/bin/
COPY --chown=root:root $JDK_VERSION /usr/local/$JDK_VERSION
COPY --chown=99:99 libSecurityStarter.so /usr/lib64/
COPY --chown=99:99 FileClient /opt/FileClient
RUN cp /opt/FileClient/lib/libkmcv3.so /usr/lib64/  \
    && chmod 755 /usr/lib64/libkmcv3.so \
    && cp /opt/FileClient/lib/3rd/libcrypto.so.3 /usr/lib64/ \
    && chmod 755 /usr/lib64/libcrypto.so.3 \
    && cp /opt/FileClient/lib/libsecurec.so /usr/lib64/ \
    && chmod 755 /usr/lib64/libsecurec.so \
    && chmod 550 /usr/lib64/libSecurityStarter.so \
    && echo "Defaults    env_keep += \"FILECLIENT_CONF_PATH FILECLIENT_LOG_PATH\"" >> /etc/sudoers
ADD common-init-${COMMON_INIT_VERSION}.tar.gz /usr/local/

RUN sed -i '/update/,$d' /etc/yum.repos.d/openEuler.repo
RUN dnf install -y python3-pip python39 openssl libatomic net-tools libuser shadow-utils openssh expect sudo iptables vim kmod libatomic dnsmasq nfs-utils iputils iproute xorg-x11-fonts-* fontconfig\
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