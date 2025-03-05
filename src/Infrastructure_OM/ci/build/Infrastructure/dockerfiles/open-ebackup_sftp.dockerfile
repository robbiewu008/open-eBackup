#指定基础镜像
FROM open-ebackup-1.0:base

# om版本
ENV SFTP_VERSION sftp_version

# 指定镜像的工作目录
WORKDIR /opt/

# 删除镜像KMC工具
RUN rm -f /usr/bin/kmcdecrypt \
    && rm -f /usr/bin/restclient

# 添加sftp安装包目录
ADD sftp-${SFTP_VERSION}.tar.gz /opt/

RUN luseradd -u 15004 -g nobody -s /sbin/nologin sftp
# Add our top code folder to python path.
ENV PYTHONPATH=/opt/sftp/package/src
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib"

WORKDIR /opt/script

RUN python3 -O -m compileall -b /opt/sftp/package/src/app \
    && find /opt/sftp/package/src/app/ -name "*.py" | xargs -i rm -rf '{}'

RUN mv /opt/sftp/package/script /opt/ \
    && chmod 750 /opt/sftp \
    && chown -R 15004:99 /opt/script \
    && chown -R 15004:99 /etc/ssh \
    && chmod 750 /opt/script \
    && cd /opt/sftp/package/requirements \
    && pip3 install --no-cache-dir * \
    && chown -R root:root /opt/sftp \
    && sed -i 's/AuthorizedKeysFile.*/AuthorizedKeysFile      \/sftp\/%u\/%u\/authorized_keys/' /etc/ssh/sshd_config \
    && echo "sftp  ALL=(root)      NOPASSWD:SETENV:/opt/sftp/package/actual_install.sh" >> /etc/sudoers \
    && echo 'Defaults    env_keep += "NODE_NAME"' >> /etc/sudoers

RUN rm -f /root/.config/pip/pip.conf

USER 15004
CMD [ "sh", "install_sftp.sh" ]