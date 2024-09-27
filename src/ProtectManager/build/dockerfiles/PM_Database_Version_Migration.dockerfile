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
FROM oceanprotect-dataprotect-1.0.rc1:base

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
    && rm -rf /usr/bin/kmcdecrypt /usr/bin/restclient

# Add our top code folder to python path.
ENV PYTHONPATH=/context/src

# Copy app
WORKDIR /context/src
COPY package/src/  .

RUN chown nobody:nobody "/context/src" \
    && chown nobody:nobody "/context"

COPY --chown=99:99 package/src/  .
RUN python3 -m py_compile /context/src/app/common/kmc_util.py /context/src/app/common/client_util.py\
    && cp -rf /context/src/app/common/__pycache__/kmc_util.*.pyc /context/src/app/common/kmc_util.pyc \
    && cp -rf /context/src/app/common/__pycache__/client_util.*.pyc /context/src/app/common/client_util.pyc \
    && rm -rf /context/src/app/common/__pycache__ /context/src/app/common/kmc_util.py /context/src/app/common/client_util.py \
    && rm -rf /context/src/common/security/kmc_util.py /context/src/common/clients/client_util.py \
    && chown nobody:nobody /context/src/app/common/kmc_util.pyc /context/src/app/common/client_util.pyc \
    && rm -rf /root/.config/pip

CMD ["python3", "-m", "app"]
