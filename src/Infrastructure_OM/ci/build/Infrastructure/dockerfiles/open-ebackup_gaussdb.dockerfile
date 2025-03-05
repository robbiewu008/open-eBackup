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
    && python3 -m pip install --no-cache-dir /opt/script/requirements/* \
    && rm -rf /opt/script/requirements \
    && python3 -m compileall -b /opt/script/ \
    && find /opt/script/ -name "*.py" | xargs -i rm -rf '{}' \
    && chown GaussDB:dbgrp /usr/local/gaussdb -R \
    && chown GaussDB:nobody /usr/local/gaussdb/install_ha.sh \
    && chown GaussDB:nobody /usr/local/gaussdb/gaussdb_kmc.py \
    && chown GaussDB:nobody /usr/local/gaussdb/gaussdb_data_mv.sh \
    && chown GaussDB:nobody /usr/local/gaussdb/auto_password_input.sh \
    && chown GaussDB:nobody /usr/local/gaussdb/gaussdb_common.py \
    && chown GaussDB:nobody /usr/local/gaussdb/gsjdbc4.jar \
    && chown -R GaussDB:nobody /usr/local/gaussdb/GaussDB_T_1.9.0-DATASYNC \
    && chown GaussDB:dbgrp /usr/bin/get.sh \
    && echo 'Defaults    env_keep += "NODE_NAME"' >> /etc/sudoers \
    && echo 'Defaults    env_keep += "POD_IP"' >> /etc/sudoers \
    && echo 'Defaults    env_keep += "DEPLOY_TYPE"' >> /etc/sudoers \
    && echo 'Defaults    env_keep += "POD_NAME"' >> /etc/sudoers \
    && echo "GaussDB ALL=(root)      NOPASSWD:/opt/script/change_permission.sh" >> /etc/sudoers \
    && echo "GaussDB ALL=(root)      NOPASSWD:/opt/script/manage_data.sh" >> /etc/sudoers \
    && echo "GaussDB ALL=(root)      NOPASSWD:/opt/script/ha_sudo.sh" >> /etc/sudoers \
    && echo "nobody ALL=(root)       NOPASSWD:/opt/script/ha_sudo.sh" >> /etc/sudoers \
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

RUN mkdir /opt/HA

COPY --chown=1000:99 HA-*-aarch64 /opt/HA

RUN ls -lR /opt/HA \
    && /opt/HA/install.sh -p /usr/local/ -u nobody -g nobody \
    && mv /opt/HA/script /usr/local/ha/ \
    && rm -rf /opt/HA \
    && mkdir /usr/local/ha/local/tmp \
    && mv /usr/local/ha/script/conf/floatIp.xml /usr/local/ha/module/harm/plugin/conf/ \
    && mv /usr/local/ha/script/conf/gaussdb.xml /usr/local/ha/module/harm/plugin/conf/ \
    && mv /usr/local/ha/script/floatIp.sh /usr/local/ha/module/harm/plugin/script/ \
    && mv /usr/local/ha/script/gaussdb.sh /usr/local/ha/module/harm/plugin/script/ \
    && mv /usr/local/ha/script/send_alarm.sh /usr/local/ha/module/hacom/plugin/script/ \
    && mv /usr/local/ha/script/conf/agent-file.xml /usr/local/ha/module/hasync/plugin/conf/ \
    && chown nobody:nobody /usr/local/ha -R \
    && chmod 750 /usr/local/ha -R \
    && chmod 550 /usr/local/ha/script -R \
    && chmod 550 /usr/local/ha/module/haarb/script -R \
    && chmod 550 /usr/local/ha/module/haarb/plugin/script -R \
    && chmod 550 /usr/local/ha/module/hacom/script -R \
    && chmod 550 /usr/local/ha/module/hacom/plugin/script -R \
    && chmod 550 /usr/local/ha/module/hacom/tools -R \
    && chmod 550 /usr/local/ha/module/hacom/bin -R \
    && chmod 550 /usr/local/ha/module/hacom/lib -R \
    && chmod 550 /usr/local/ha/module/hamon/script -R \
    && chmod 550 /usr/local/ha/module/hamon/plugin/script -R \
    && chmod 550 /usr/local/ha/module/hamon/bin -R \
    && chmod 550 /usr/local/ha/module/harm/script -R \
    && chmod 550 /usr/local/ha/module/harm/plugin/script -R \
    && chmod 550 /usr/local/ha/module/hasync/script -R \
    && chmod 550 /usr/local/ha/module/hasync/plugin/script -R \
    && chmod 550 /usr/local/ha/uninstall.sh \
    && chmod 640 /usr/local/ha/module/haarb/conf/haarb.xml \
    && chmod 640 /usr/local/ha/module/harm/conf/harm.xml \
    && chmod 640 /usr/local/ha/module/harm/plugin/conf/* \
    && chmod 640 /usr/local/ha/module/hasync/conf/hasync.xml \
    && chmod 640 /usr/local/ha/module/hasync/plugin/conf/* \
    && chmod 640 /usr/local/ha/version \
    && chmod 750 /usr/local/ha/script/conf \
    && chmod 640 /usr/local/ha/script/conf/* \
    && chmod 750 `find /usr/local/ha/module -type d` \
    && rm -f /root/.config/pip/pip.conf

ENV GAUSSDATA=/usr/local/gaussdb/data
ENV PATH=/usr/local/gaussdb/app/bin:$PATH
ENV GAUSSHOME=/usr/local/gaussdb/app
ENV GAUSSLOG=/usr/local/gaussdb/data/pg_log
# 配置MALLOC_CONF 环境变量
ENV MALLOC_CONF="dirty_decay_ms:0,muzzy_decay_ms:0,lg_tcache_max:15"
# 指定端口号
EXPOSE gaussdb_port
CMD [ "sh", "install_opengauss.sh" ]
