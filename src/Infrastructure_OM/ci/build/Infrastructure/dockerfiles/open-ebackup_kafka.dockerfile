# 基础镜像
FROM open-ebackup-1.0:base

# kafka版本
ENV KAFKA_VERSION kafka_version

# 在构建镜像时，指定镜像的工作目录，之后的命令都是基于此工作目录，如果不存在，则会创建目录
WORKDIR /usr/local/kafka

# 删除镜像KMC工具
RUN rm -f /usr/bin/restclient
RUN luseradd -u 99 -g nobody -s /sbin/nologin nobody

COPY --chown=99:99 kafka-${KAFKA_VERSION} /usr/local/kafka/kafka-${KAFKA_VERSION}/
COPY --chown=99:99 kafka-scripts /opt/script/
RUN chown 99:99 /usr/local/kafka \
    && chmod 750 /usr/local/kafka \
    && chown 99:99 /usr/local/kafka/kafka-${KAFKA_VERSION} \
    && chown 99:99 /opt/script \
    && chmod 550 /opt/script

WORKDIR /usr/local/kafka/kafka-${KAFKA_VERSION}

RUN mv "/opt/script/mount_oper.sh" "/opt" \
    && chown root:root /opt/mount_oper.sh \
    && python3 -m compileall -b /opt/script/kmc/ \
    && find /opt/script/kmc/ -name "*.py" | xargs -i rm -rf '{}' \
    && chown -R 99:99 /opt/script/kmc/
RUN echo "nobody   ALL=(ALL) NOPASSWD:/opt/mount_oper.sh" >> /etc/sudoers \
    && echo 'Defaults    env_keep += "NODE_NAME"' >> /etc/sudoers

RUN rpm -qa | grep ^libxcrypt-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^glibc-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-extra-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^cpp-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^make-[0-9] | xargs -i rpm -e {} --nodeps

ENV PYTHONPATH=/opt/script

USER 99
RUN pip3 install --user -i https://cmc.centralrepo.rnd.huawei.com/pypi/simple \
    --trusted-host cmc.centralrepo.rnd.huawei.com --no-cache-dir \
    -r /opt/script/requirements.txt

USER root
RUN rm -f /root/.config/pip/pip.conf
USER 99
# 暴露 9092 端口
EXPOSE kafka_port

WORKDIR /opt/script
CMD ["sh", "install_kafka.sh"]
