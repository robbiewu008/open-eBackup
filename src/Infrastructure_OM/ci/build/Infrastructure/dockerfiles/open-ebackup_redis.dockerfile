#指定基础镜像
FROM open-ebackup-1.0:base

# redis版本
ENV REDIS_VERSION redis_version

# 在构建镜像时，指定镜像的工作目录，之后的命令都是基于此工作目录，如果不存在，则会创建目录
WORKDIR /usr/local/redis

# 删除镜像KMC工具
RUN rm -f /usr/bin/kmcdecrypt \
    && rm -f /usr/bin/restclient

RUN luseradd -u 99 -g nobody -s /sbin/nologin nobody

#将redis中的redis配置文件拷贝到容器的/usr/local/redis目录中
ADD redis-${REDIS_VERSION}.tar.gz /usr/local/redis
COPY --chown=99:99 redis-scripts /opt/script/

RUN chown -R 99:99 /usr/local/redis \
    && chmod 750 /usr/local/redis \
    && chown -R 99:99 /opt/script \
    && chmod 550 /opt/script

WORKDIR /usr/local/redis/redis-${REDIS_VERSION}

RUN mv "/opt/script/mount_oper.sh" "/opt" \
    && chown root:root /opt/mount_oper.sh \
    && python3 -m compileall -b /opt/script/kmc/ \
    && find /opt/script/kmc/ -name "*.py" | xargs -i rm -rf '{}' \
    && chown -R 99:99 /opt/script/kmc/

RUN echo "nobody   ALL=(ALL) NOPASSWD:/opt/mount_oper.sh" >> /etc/sudoers \
    && echo 'Defaults    env_keep += "NODE_NAME"' >> /etc/sudoers

RUN rm -f /root/.config/pip/pip.conf

ENV PYTHONPATH=/opt/script

# VOLUME ["/usr/local/redis/logs"]

# 暴露 6369 端口
EXPOSE redis_port

USER 99
WORKDIR /opt/script
CMD ["sh", "install_redis.sh"]