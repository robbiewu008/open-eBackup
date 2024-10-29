FROM openeuler-22.03-lts:latest

WORKDIR /usr/local

RUN dnf -y install python3 python3-pip python3-devel shadow-utils libuser tar gcc gcc-c++ make cmake ccache libffi-devel vim-enhanced \
                   libatomic java-1.8.0-openjdk libstdc++-static xz maven nodejs wget ant hostname patch openssl binutils findutils \
                   glibc-devel readline-devel make libaio  perl openssl-devel libaio-devel flex bison ncurses-devel glibc-devel patch \
                   openeuler-lsb readline-devel sudo pigz rpcgen unzip \
     && dnf clean all \
     && rm -rf /var/cache/dnf \
     && ln -s /usr/bin/python3 /usr/bin/python


ENV JAVA_HOME /usr/lib/jvm/java-1.8.0-openjdk
ENV PATH $JAVA_HOME/bin:$PATH
ENV LANG C.utf8
ENV NODE_HOME /open-eBackup/open-eBackup-bin/node-v18.20.1-linux-arm64/
ENV PATH $NODE_HOME/bin:$PATH
