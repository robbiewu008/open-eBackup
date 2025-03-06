#!/bin/bash
product_version="1.0"
export product_name="open-ebackup"

slim_image="euleros-base-aarch64-V200R012C00SPC300B550.tar.xz"
slim_image_name="euleros-base-aarch64"
slim_image_tag="V200R012C00SPC300B550"
PACKAGE1="EulerOS_Server_V200R012C00SPC500B750.tar.gz"
PACKAGE2="devel_tools.tar.gz"
base_euler_yum_url="https://cmc-ctu-artifactory.cmc.tools.huawei.com/artifactory/cmc-software-release/DPA/OS%20DEPENDENCY/1.6.0//arm/euler2.12/"
base_openeuler_yum_url="https://mirrors.tools.huawei.com/openeuler/openEuler-20.03-LTS/everything/aarch64/"
jdk_version="jre1.8.0_432"
jdk_package="jre-8u432-linux-aarch64.tar.gz"
KillerOfPlaintextEcho="KillerOfPlaintextEcho-0.3.0.zip"

zookeeper_version="3.8.1"
zookeeper_port="2181"
kafka_version="2.12-3.5.0"
kafka_port="9092"
redis_version="6.2.14"
redis_port="6369"
elasticsearch_version="7.10.2"
elasticsearch_port="9200"
gaussdb_version="505.1.0.SPC0900.B003"
gaussdb_port="6432"
ha_version="1.3.0"
PKG_TYPE="ARM_64"

debug_add_rpm_str='["augeas","binutils","bzip2-devel","cifs-utils","cyrus-sasl","dbus","device-mapper-event",\
        "dhcp","dnf-plugins-core","dnsmasq","dosfstools","e2fsprogs-devel","expat-devel","gcc","gdbm-devel",\
        "genisoimage","iproute","iptables","iputils","ipxe-roms-qemu","jansson","jbigkit-libs",\
        "json-glib","libaio","libconfig","libdrm","libepoxy","libffi-devel","libiscsi","libjpeg-turbo",\
        "libogg","libpciaccess","libssh2","libtiff","libusal","libxslt","lsscsi","lvm2","lzop","make","mdadm","mesa-libgbm","nspr",\
        "nss","nss-softokn","nss-util","numad","parted","passwd","pciutils","policycoreutils","psmisc",\
        "python3-devel","qemu-img","radvd","rdma-core","readline-devel","rsync","selinux-policy",\
        "glusterfs","kernel","libvirt-daemon","libvirt-daemon-driver-interface","libvirt-daemon-driver-network",\
        "libvirt-daemon-driver-nodedev","libvirt-daemon-driver-nwfilter","libvirt-daemon-driver-qemu",\
        "libvirt-daemon-driver-secret","libvirt-daemon-driver-storage","libvirt-daemon-driver-storage-core",\
        "libvirt-daemon-driver-storage-disk","libvirt-daemon-driver-storage-iscsi","libvirt-daemon-driver-storage-logical",\
        "libvirt-daemon-driver-storage-mpath","libvirt-daemon-driver-storage-scsi","libvirt-daemon-kvm","libvirt-libs",\
        "sqlite-devel","squashfs-tools","systemd-container","usbredir","userspace-rcu","xfsprogs","yajl","ndsend","libsecurec","python3-pytz","ethtool","sudo","less"]'

debug_remove_rpm_str='["kbd-legacy","trousers","kbd-misc","shared-mime-info","libxkbcommon"]'
debug_remove_file_str='["/usr/share/bash-completion","/usr/share/doc","/usr/share/i18n",\
                 "/usr/share/info","/usr/share/licenses","/usr/share/man","/usr/share/zoneinfo/Antarctica",\
                 "/usr/share/zoneinfo/Arctic","/usr/share/zoneinfo/[B-D]*","/usr/share/zoneinfo/E[A-s]*",\
                 "/usr/share/zoneinfo/[F-O]*","/usr/share/zoneinfo/P[A-Z]*","/usr/share/zoneinfo/P[b-z]*",\
                 "/usr/share/zoneinfo/[Q-y]*","/usr/bin/objdump"]'

release_add_rpm_str='["augeas","binutils","bzip2-devel","cifs-utils","cyrus-sasl","dbus","device-mapper-event",\
        "dhcp","dnf-plugins-core","dnsmasq","dosfstools","e2fsprogs-devel","expat-devel","gcc","gdbm-devel",\
        "genisoimage","iproute","iptables","iputils","ipxe-roms-qemu","jansson","jbigkit-libs",\
        "json-glib","libaio","libconfig","libdrm","libepoxy","libffi-devel","libiscsi","libjpeg-turbo",\
        "libogg","libpciaccess","libssh2","libtiff","libusal","libxslt","lsscsi","lvm2","lzop","make","mdadm","mesa-libgbm","nspr",\
        "nss","nss-softokn","nss-util","numad","parted","passwd","pciutils","policycoreutils","psmisc",\
        "python3-devel","qemu-img","radvd","rdma-core","readline-devel","rsync","selinux-policy",\
        "glusterfs","kernel","libvirt-daemon","libvirt-daemon-driver-interface","libvirt-daemon-driver-network",\
        "libvirt-daemon-driver-nodedev","libvirt-daemon-driver-nwfilter","libvirt-daemon-driver-qemu",\
        "libvirt-daemon-driver-secret","libvirt-daemon-driver-storage","libvirt-daemon-driver-storage-core",\
        "libvirt-daemon-driver-storage-disk","libvirt-daemon-driver-storage-iscsi","libvirt-daemon-driver-storage-logical",\
        "libvirt-daemon-driver-storage-mpath","libvirt-daemon-driver-storage-scsi","libvirt-daemon-kvm","libvirt-libs",\
        "sqlite-devel","squashfs-tools","systemd-container","usbredir","userspace-rcu","xfsprogs","yajl","ndsend","libsecurec","python3-pytz","ethtool","sudo"]'

release_remove_rpm_str='["kbd-legacy","trousers","kbd-misc","shared-mime-info","libxkbcommon"]'
release_remove_file_str='["/usr/share/bash-completion","/usr/share/doc","/usr/share/i18n",\
                 "/usr/share/info","/usr/share/licenses","/usr/share/man","/usr/share/zoneinfo/Antarctica",\
                 "/usr/share/zoneinfo/Arctic","/usr/share/zoneinfo/[B-D]*","/usr/share/zoneinfo/E[A-s]*",\
                 "/usr/share/zoneinfo/[F-O]*","/usr/share/zoneinfo/P[A-Z]*","/usr/share/zoneinfo/P[b-z]*",\
                 "/usr/share/zoneinfo/[Q-y]*","/usr/bin/objdump"]'
