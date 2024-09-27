# 指定基础镜像
FROM oceanprotect-dataprotect-1.0.rc1:base

WORKDIR /opt/install
ADD DataProtect_client /opt/install/DataProtect_client

WORKDIR /opt/install/DataProtect_client
RUN sh install.sh \
    && rm -rf /opt/install \
    && yum install krb5-workstation -y \
    && yum install expect -y \
    && rpm -qa | grep ^libxcrypt-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^glibc-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^gcc-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-extra-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^cpp-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^make-[0-9] | xargs -i rpm -e {} --nodeps \
    && yum clean all

WORKDIR /opt/DataBackup/ProtectClient/ProtectClient-E
CMD ["/bin/bash", "-c", "/opt/DataBackup/ProtectClient/ProtectClient-E/bin/internal_run.sh"]