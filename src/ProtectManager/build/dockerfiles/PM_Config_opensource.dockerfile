ARG VERSION_3RD
FROM open-ebackup-1.0:base

WORKDIR /usr/local/lib
COPY package/3rd/libpq.so.5.5    .
COPY package/3rd/libcrypto.so    .
COPY package/3rd/libssl.so    .
COPY package/3rd/psycopg2    /usr/local/lib64/python3.9/site-packages/psycopg2
RUN ln -s libpq.so.5.5 libpq.so.5 && ln -s libpq.so.5 libpq.so
COPY package/3rd/librdkafka_user_local/    /usr/local/
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib"
RUN ldconfig

# Build required packages
WORKDIR /wheels

# Add appoint packages
COPY package/3rd/ /wheels/

RUN luseradd -u 99 -g nobody -s /sbin/nologin nobody
RUN yum install -y python3-devel

RUN yum install -y gcc \
&& pip3 install --no-index --find-links=./ --no-cache-dir setuptools==65.6.3 \
&& pip3 install --no-index --find-links=./ --no-cache-dir wheel==0.44.0 \
&& sh prepare_env.sh && rm -rf /wheels/ && yum remove -y gcc && yum clean all \
&& chmod -R 755 "/usr/local/lib" \
&& chmod -R 755 "/usr/lib/python3.9" \
&& chmod -R 755 "/usr/local/lib64/python3.9" \
&& chmod -R 755 "/usr/lib64/python3.9"

RUN rpm -qa | grep ^libxcrypt-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^glibc-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^gcc-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-extra-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^cpp-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^make-[0-9] | xargs -i rpm -e {} --nodeps

WORKDIR /opt/OceanStor/100P/ProtectManager/PM_Config/app
COPY src/app/ /opt/OceanStor/100P/ProtectManager/PM_Config/app/
WORKDIR /opt/OceanStor/100P/ProtectManager/PM_Config/scripts
COPY scripts/ /opt/OceanStor/100P/ProtectManager/PM_Config/scripts/

WORKDIR /opt/OceanStor/100P/ProtectManager/PM_Config

RUN mkdir -p "/opt/scripts" \
    && cp "/opt/OceanStor/100P/ProtectManager/PM_Config/scripts/export.sh" "/opt/scripts" \
    && cp "/opt/OceanStor/100P/ProtectManager/PM_Config/scripts/common.sh" "/opt/scripts" \
    && chown -R root:nobody "/opt/scripts" \
    && chmod 750 "/opt/scripts" \
    && echo "nobody  ALL=(root)      NOPASSWD:/opt/scripts/export.sh *" >> /etc/sudoers \
    && echo "nobody  ALL=(root)      NOPASSWD:/opt/scripts/common.sh *" >> /etc/sudoers \
    && chmod 550 "/opt/scripts/common.sh" \
    && chmod 550 "/opt/scripts/export.sh" \
    && chown -R 99:99 /opt/OceanStor/100P/ProtectManager/PM_Config \
    && chmod -R 750 /opt/OceanStor/100P/ProtectManager/PM_Config
# Add our top code folder to python path.
ENV PYTHONPATH=/opt/OceanStor/100P/ProtectManager/PM_Config

USER 99
CMD ["python3","-m","app"]