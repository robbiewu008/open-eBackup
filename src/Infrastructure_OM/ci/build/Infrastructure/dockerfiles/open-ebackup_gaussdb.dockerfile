#指定基础镜像
FROM open-ebackup-1.0:base

# GAUSSDB版本
ENV GAUSSDB_VERSION gaussdb_version
ENV LD_LIBRARY_PATH='/usr/local/gaussdb/lib':$LD_LIBRARY_PATH

# 指定镜像的工作目录
WORKDIR /usr/local/gaussdb

# 删除镜像KMC工具
RUN rm -f /usr/bin/kmcdecrypt

# 添加gaussdb安装包目录
COPY gaussdb-${GAUSSDB_VERSION} /usr/local/gaussdb

ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib"

RUN chmod 750 /usr/local/gaussdb

RUN groupadd -g 1000 dbgrp \
    && luseradd nobody -s /sbin/nologin  -u 99 -g 99 \
    && useradd -g nobody -s /sbin/nologin -u 1000 GaussDB \
    && usermod -a -G dbgrp GaussDB \
    && mv script /opt/ \
    && chown -R root:root /opt/script \
    && python3 -m compileall -b /opt/script/ \
    && find /opt/script/ -name "*.py" | xargs -i rm -rf '{}' \
    && chown GaussDB:dbgrp /usr/local/gaussdb -R \
    && chown GaussDB:nobody /usr/local/gaussdb/gaussdb_kmc.py \
    && chown GaussDB:nobody /usr/local/gaussdb/gaussdb_common.py \
    && chown GaussDB:dbgrp /usr/bin/get.sh \
    && echo 'Defaults    env_keep += "NODE_NAME"' >> /etc/sudoers \
    && echo 'Defaults    env_keep += "POD_IP"' >> /etc/sudoers \
    && echo 'Defaults    env_keep += "DEPLOY_TYPE"' >> /etc/sudoers \
    && echo 'Defaults    env_keep += "POD_NAME"' >> /etc/sudoers \
    && echo "GaussDB ALL=(root)      NOPASSWD:/opt/script/change_permission.sh" >> /etc/sudoers \
    && echo "GaussDB ALL=(root)      NOPASSWD:/usr/sbin/ntpd" >> /etc/sudoers \
    && echo "nobody ALL=(root)   NOPASSWD:/usr/sbin/ip" >> /etc/sudoers \
    && echo "interface ignore wildcard" >> /etc/ntp.conf \
    && echo "interface listen eth0" >> /etc/ntp.conf \
    && echo "server 127.127.1.0 prefer" >> /etc/ntp.conf

USER GaussDB:dbgrp

RUN sh /usr/local/gaussdb/install.sh --mode single -D /usr/local/gaussdb/data  -R /usr/local/gaussdb/app -P "-w gaussdb@123" -C port=6432 --start \
    && chmod 750 /usr/local/gaussdb/app/bin \
    && chmod 750 /usr/local/gaussdb/app/lib \
    && chmod 750 /usr/local/gaussdb/app/bin/gsql \
    && chmod 750 /usr/local/gaussdb/app/lib/libpq.so.5.5 \
    && chmod 750 /usr/local/gaussdb/app/lib/libssl.so \
    && chmod 750 /usr/local/gaussdb/app/lib/libcrypto.so \
    && chmod 750 /usr/local/gaussdb/app/lib/libcjson.so.1 \
    && chmod 750 /usr/local/gaussdb/app

USER root

# 添加运维账户
RUN groupadd -g 61100 GaussOp \
    && useradd -g 61100 -s /bin/bash -u 61100 GaussOp \
    && usermod -a -G dbgrp GaussOp \
    && echo "GaussOp ALL=(root)      NOPASSWD:/opt/script/gauss_operation.sh" >> /etc/sudoers

# 设置GaussDB通过su切换用户GaussOp免密
RUN sed -i '2a\auth       sufficient   pam_succeed_if.so use_uid user ingroup GaussOp' /etc/pam.d/su \
    && sed -i '2a\auth       [success=ignore default=1] pam_succeed_if.so user = GaussOp' /etc/pam.d/su \
    && sed -i 's/interface listen eth0/interface listen cbri1601/' /etc/ntp.conf \
    && usermod -aG GaussOp GaussDB

RUN rpm -qa | grep ^libxcrypt-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^glibc-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-extra-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^cpp-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^make-[0-9] | xargs -i rpm -e {} --nodeps \
    && mkdir /opt/HA

ENV GAUSSDATA=/usr/local/gaussdb/data
ENV PATH=/usr/local/gaussdb/app/bin:$PATH
ENV GAUSSHOME=/usr/local/gaussdb/app
ENV GAUSSLOG=/usr/local/gaussdb/data/pg_log
# 指定端口号
EXPOSE gaussdb_port
CMD [ "sh", "install_opengauss.sh" ]
