## 第一阶段
#指定基础镜像
FROM open-ebackup-1.0:base

# om版本
ENV OM_VERSION 1.6.RC2.010
ENV PSYCOPG_VERSION 2_9_1

# 指定镜像的工作目录
WORKDIR /opt/om

# 删除镜像KMC工具
RUN rm -f /usr/bin/kmcdecrypt

# 添加om安装包目录
ADD om-${OM_VERSION}.tar.gz /opt/om

WORKDIR /opt/om/package

RUN cd /opt/om/package/3rd \
    && tar xzf psycopg2.tar.gz


## 第二阶段
#指定基础镜像
FROM open-ebackup-1.0:base

# om版本
ENV SQLALCHEMY_VERSION 1.4.49

# Add our top code folder to python path.
ENV PYTHONPATH=/opt/om/package/src
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib"

# 指定镜像的工作目录
WORKDIR /opt/om

# 从第一阶段中拷贝package
COPY --from=0 /opt/om/package /opt/om/package

WORKDIR /opt/om/package
RUN luseradd -u 99 -g nobody -s /sbin/nologin nobody
RUN python3 -O -m compileall -b /opt/om/package/src/app \
    && find /opt/om/package/src/app/ -name "*.py" | xargs -i rm -rf '{}'

# 安装SQLAlchemy 依赖greenlet、 importlib-metadata
RUN cd /opt/om/package/3rd/requirements \
    && ls | grep greenlet | xargs -i pip3 install {} \
    && ls | grep zipp | xargs -i pip3 install {} \
    && ls | grep importlib | grep metadata | xargs -i pip3 install {} \
    && cd -

RUN cd /opt/om/package/3rd \
    && tar xzf SQLAlchemy-${SQLALCHEMY_VERSION}.tar.gz \
    && cd SQLAlchemy-${SQLALCHEMY_VERSION} \
    && python3 setup.py install

COPY --from=0 /opt/om/package/3rd/psycopg2 /usr/local/lib64/python3.9/site-packages/psycopg2

RUN cd /opt/om/package/3rd \
    && cp libpq.so.5.5 /usr/local/lib \
    && cp libssl.so /usr/local/lib \
    && cp libcrypto.so /usr/local/lib \
    && cd /usr/local/lib \
    && ln -sf libpq.so.5.5 libpq.so.5 && ln -sf libpq.so.5 libpq.so \
    && ldconfig \
    && cd /opt/om/package/3rd/requirements \
    && pip3 install *.whl \
    && chmod -R 755 "/usr/local/lib/python3.9" \
    && chmod -R 755 "/usr/lib/python3.9" \
    && chmod -R 755 "/usr/local/lib64/python3.9" \
    && chmod -R 755 "/usr/lib64/python3.9" \
    && find "/opt/om/package/" -type f -name "*.o" | xargs rm -f \
    && echo "nobody  ALL=(root)      NOPASSWD:/opt/script/export_log.sh" >> /etc/sudoers \
    && echo "nobody  ALL=(root)      NOPASSWD:/opt/script/service_log_process.sh" >> /etc/sudoers \
    && echo "nobody  ALL=(root)      NOPASSWD:/opt/script/change_permission.sh" >> /etc/sudoers \
    && echo 'Defaults    env_keep += "NODE_NAME"' >> /etc/sudoers \
    && echo 'Defaults    env_keep += "DEPLOY_TYPE"' >> /etc/sudoers \
    && mv /opt/om/package/script /opt/ \
    && chown -R root:root /opt/script \
    && chmod 750 /opt/om \
    && chmod 750 /opt/om/package \
    && chown -R 99:99 /opt/om \
    && mv /opt/om/package/upgrade /opt/ \
    && useradd -M -N -s /sbin/nologin -u 1000 -g nobody GaussDB \
    && useradd -M -N -s /sbin/nologin -u 65500 -g nobody OceanProtectNginx

RUN echo "/usr/local/lib" >> /etc/ld.so.conf \
    && ldconfig \
    && chown nobody:nobody /usr/local/lib/libpq.so.5.5 \
    && chown nobody:nobody /usr/local/lib/libpq.so.5 \
    && chown nobody:nobody /usr/local/lib/libpq.so \
    && chown nobody:nobody /usr/local/lib/libssl.so \
    && chown nobody:nobody /usr/local/lib/libcrypto.so

RUN rpm -qa | grep ^libxcrypt-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^glibc-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-extra-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^cpp-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^make-[0-9] | xargs -i rpm -e {} --nodeps \
    && rm -f /root/.config/pip/pip.conf

CMD [ "sh", "run.sh" ]