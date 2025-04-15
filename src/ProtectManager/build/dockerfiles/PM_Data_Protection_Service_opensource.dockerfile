# ---------------------------------------------------------------------
#
# $file: Dockerfile
# $author: Protect Manager
#
# ---------------------------------------------------------------------

# ---------------------------------------------------------------------
# Pull requirements python packages
# ---------------------------------------------------------------------

# FROM test:latest
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

RUN luseradd -u 15013 -g nobody -s /sbin/nologin pm_protection_service
RUN yum install -y python3-devel
RUN yum install -y gcc && sh prepare_env.sh && rm -rf /wheels/ && yum remove -y gcc && yum clean all \
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

RUN echo 'umask 0027' >> /etc/bashrc \
    && rm -rf /usr/bin/kmcdecrypt

# Add our top code folder to python path.
ENV PYTHONPATH=/context/src

# Copy app
WORKDIR /context/src
RUN chmod 750 /context/src \
    && chown -R 15013:99 /context/src

COPY --chown=15013:99 package/src/ .
RUN chown 15013:nobody /context/src/app/common/security/kmc_util.pyc /context/src/app/common/clients/client_util.pyc /context/src/app/__main__.pyc \
    && echo "/usr/local/lib" >> /etc/ld.so.conf \
    && ldconfig \
    && touch "/etc/timezone" \
    && chown 15013:nobody -R "/etc/timezone" \
    && mkdir "logs" \
    && chown 15013:nobody -R "logs" \
    && mkdir -m 750 "/script" \
    && chown root:nobody "/script" \
    && echo "pm_protection_service  ALL=(root)      NOPASSWD:/script/mount_oper.sh  * * *" >> /etc/sudoers \
    && mv "/context/src/mount_oper.sh" "/script" \
    && chmod 550 "/script/mount_oper.sh" \
    && chown root:root "/script/mount_oper.sh" \
    && rm -rf /root/.config/pip \
    && rm -rf /etc/localtime


USER 15013
CMD ["sh", "app.sh"]
