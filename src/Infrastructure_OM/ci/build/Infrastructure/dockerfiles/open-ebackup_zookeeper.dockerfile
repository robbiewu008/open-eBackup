# 基础镜像
FROM open-ebackup-1.0:base

# zookeeper版本
ENV ZOOKEEPER_VERSION zookeeper_version

# 在构建镜像时，指定镜像的工作目录，之后的命令都是基于此工作目录，如果不存在，则会创建目录
WORKDIR /usr/local/zookeeper

# 删除镜像KMC工具
RUN rm -f /usr/bin/kmcdecrypt \
    && rm -f /usr/bin/restclient

RUN luseradd -u 99 -g nobody -s /sbin/nologin nobody
RUN mkdir /usr/local/zookeeper/data \
    && chmod 750 /usr/local/zookeeper/data \
    && chown -R 99:nobody /usr/local/zookeeper/data

COPY --chown=99:99 zookeeper-${ZOOKEEPER_VERSION} /usr/local/zookeeper/zookeeper-${ZOOKEEPER_VERSION}/
RUN chown 99:99 /usr/local/zookeeper \
    && chmod 750 /usr/local/zookeeper \
    && chown 99:99 /usr/local/zookeeper/zookeeper-${ZOOKEEPER_VERSION} \
    && chmod 750 /usr/local/zookeeper/zookeeper-${ZOOKEEPER_VERSION}

WORKDIR /usr/local/zookeeper/zookeeper-${ZOOKEEPER_VERSION}

RUN [ -d "/opt/script" ] || mkdir -p "/opt/script" \
    && mv check_health.sh "/opt/script" \
    && mv install_zookeeper.sh "/opt/script" \
    && chown -R 99:99 "/opt/script" \
    && chmod -R 550 "/opt/script" \
    && mv mount_oper.sh /opt \
    && chown root:root /opt/mount_oper.sh \
    && chmod 500 /opt/mount_oper.sh

RUN echo "nobody   ALL=(ALL) NOPASSWD:/opt/mount_oper.sh" >> /etc/sudoers \
    && echo 'Defaults    env_keep += "NODE_NAME"' >> /etc/sudoers

RUN rm -f /root/.config/pip/pip.conf

USER 99
# 暴露 2181 端口
EXPOSE zookeeper_port

WORKDIR /opt/script
CMD ["sh", "install_zookeeper.sh"]