#指定基础镜像
FROM open-ebackup-1.0:base

# es版本
ENV ELASTICSEARCH_VERSION elasticsearch_version

#在构建镜像时，指定镜像的工作目录，之后的命令都是基于此工作目录，如果不存在，则会创建目录
WORKDIR /usr/local/elasticsearch

# 删除镜像KMC工具
RUN rm -f /usr/bin/kmcdecrypt \
    &&  rm -f /usr/bin/restclient

#安装es环境
RUN luseradd -u 99 -g nobody -s /sbin/nologin nobody
COPY --chown=99:99 elasticsearch-${ELASTICSEARCH_VERSION} /usr/local/elasticsearch/elasticsearch-${ELASTICSEARCH_VERSION}
RUN chown 99:99 /usr/local/elasticsearch \
    && chmod 750 /usr/local/elasticsearch \
    && chown 99:99 /usr/local/elasticsearch/elasticsearch-${ELASTICSEARCH_VERSION}
WORKDIR /usr/local/elasticsearch/elasticsearch-${ELASTICSEARCH_VERSION}

RUN [ -d "/opt/script" ] || mkdir -p "/opt/script" \
    && mv check_health.sh "/opt/script" \ 
    && mv install_elasticsearch.sh "/opt/script" \
    && mv net_plane_ip.py "/opt/script" \
    && mv check_elasticsearch_readiness.sh "/opt/script" \
    && chown -R 99:99 "/opt/script" \
    && chmod -R 550 "/opt/script" \
    && mv mount_oper.sh "/opt" \
    && chown root:root /opt/mount_oper.sh \
    && chmod 500 /opt/mount_oper.sh
RUN echo "nobody   ALL=(ALL) NOPASSWD:/opt/mount_oper.sh" >> /etc/sudoers \
    && echo 'Defaults    env_keep += "NODE_NAME"' >> /etc/sudoers

RUN rpm -qa | grep ^libxcrypt-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^glibc-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-extra-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^cpp-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^make-[0-9] | xargs -i rpm -e {} --nodeps \
    # 普通容器中，该脚本里的if -x 语句不生效，暂未定位
    && sed -i '/! -x/,+4d' /usr/local/elasticsearch/elasticsearch-${ELASTICSEARCH_VERSION}/bin/elasticsearch-env \
    && rm -f /root/.config/pip/pip.conf


USER 99
# 暴露 9200 端口
EXPOSE elasticsearch_port
WORKDIR /opt/script
CMD [ "sh", "install_elasticsearch.sh"]
