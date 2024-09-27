FROM open-ebackup-1.0:base
WORKDIR /public_cbb
COPY postgresql_dep/bin/ /public_cbb/postgresql_dep/bin/
COPY postgresql_dep/include/ /public_cbb/postgresql_dep/include/
COPY postgresql_dep/lib/ /public_cbb/postgresql_dep/lib/
COPY postgresql_dep/share/ /public_cbb/postgresql_dep/share/
COPY lib /public_cbb/lib
COPY psycopg2 /public_cbb/psycopg2
ENV PATH="$PATH:/public_cbb/postgresql_dep/bin:/public_cbb/postgresql_dep/include:/public_cbb/postgresql_dep/lib:/public_cbb/postgresql_dep/share"
WORKDIR /public_cbb

FROM open-ebackup-1.0:base
WORKDIR /public_cbb
COPY setup.py /public_cbb/
COPY public_cbb /public_cbb/public_cbb/
COPY public_cbb/config/.env /opt/.env
COPY public_cbb/script/mount_oper.sh /usr/bin/mount_oper.sh
RUN pip3 install -i https://cmc.centralrepo.rnd.huawei.com/artifactory/pypi-central-repo/simple \
    --trusted-host cmc.centralrepo.rnd.huawei.com --no-cache-dir \
    -r public_cbb/requirements.txt && python3 setup.py install && \
    sed -i 's/v = connection.execute("select version()").scalar()/v = \"PostgreSQL 15.2\"/g' \
    /usr/local/lib64/python3.9/site-packages/sqlalchemy/dialects/postgresql/base.py && rm -rf /public_cbb && \
    cd /usr/local/lib/python*/site-packages/public_cbb*/public_cbb/security/pwd_manage/ && \
    mv __pycache__/kmc_manage.*.pyc kmc_manage.pyc && \
    mv __pycache__/cert_manage.*.pyc cert_manage.pyc && \
    mv __pycache__/kmc_util.*.pyc kmc_util.pyc && \
    rm -rf kmc_manage.py cert_manage.py kmc_util.py && \
    rpm -qa | grep ^libxcrypt-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^glibc-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^gcc-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-extra-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^cpp-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^make-[0-9] | xargs -i rpm -e {} --nodeps \
    && rm -f /root/.config/pip/pip.conf
COPY --from=0 /public_cbb/psycopg2 /usr/local/lib64/python3.9/site-packages/psycopg2
COPY --from=0 /public_cbb/lib /usr/lib64
