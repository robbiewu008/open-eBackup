#!/bin/bash

base_euler_yum_url="https://cmc-ctu-artifactory.cmc.tools.huawei.com/artifactory/cmc-software-release/DPA/OS%20DEPENDENCY/1.6.0//arm/euler2.12/"

# 待添加的RPM列表
RPM_EULER_LIST=("binutils" "bzip2-devel" "cifs-utils" "cyrus-sasl" "dbus" "device-mapper-event" \
        "dhcp" "dnf-plugins-core" "dnsmasq" "dosfstools" "e2fsprogs-devel" "expat-devel" "gdbm-devel" \
        "genisoimage" "iproute" "iptables" "iputils" "ipxe-roms-qemu" "jansson" "jbigkit-libs" \
        "json-glib" "libaio" "libconfig" "libdrm" "libepoxy" "libffi-devel" "libiscsi" "libjpeg-turbo" \
        "libogg" "libpciaccess" "libssh2" "libtiff" "libusal" "libxslt" "lsscsi" "lvm2" "lzop" "make" "mdadm" "mesa-libgbm" "nspr" \
        "nss" "nss-softokn" "nss-util" "parted" "passwd" "pciutils" "policycoreutils" "psmisc" \
        "python3-devel" "qemu-img" "radvd" "rdma-core" "readline-devel" "rsync" "selinux-policy" \
        "glusterfs" "kernel" "sqlite-devel" "squashfs-tools" "systemd-container" "usbredir" "userspace-rcu" "xfsprogs" "yajl" \
        "ndsend" "libsecurec" "python3-pytz" "ethtool" "sudo" "python3-pip" "shadow" "libuser" \
        "libvirt-daemon" "libvirt-daemon-driver-interface" "libvirt-daemon-driver-network" \
        "libvirt-daemon-driver-nodedev" "libvirt-daemon-driver-nwfilter" "libvirt-daemon-driver-qemu" \
        "libvirt-daemon-driver-secret" "libvirt-daemon-driver-storage" "libvirt-daemon-driver-storage-core" \
        "libvirt-daemon-driver-storage-disk" "libvirt-daemon-driver-storage-iscsi" "libvirt-daemon-driver-storage-logical" \
        "libvirt-daemon-driver-storage-mpath" "libvirt-daemon-driver-storage-scsi" "libvirt-daemon-kvm" "libvirt-libs" \
        "acl" "cronie" "cryptsetup" "curl" "dbus-glib" "deltarpm" "fipscheck" "libatomic" "libedit" "libfastjson"
        "libnet" "mariadb-connector-c" "net-tools" "net-snmp-libs" "openssh" "openssh-clients" "openssh-server" "openssl-pkcs11"
        "python-pip-wheel" "python-setuptools" "python3-unbound" "python3-unversioned-command" "rsyslog" "sg3_utils" "tar" "wget" \
        "expect" "ndisc6" "unzip" "open-isns" "open-iscsi" "ntp" "sysstat")

# 待删除的RPM列表
REMOVE_RPM_LIST=("kbd-legacy" "kbd-misc" "libxkbcommon" "trousers" "dbus-glib" "deltarpm")
# 待删除的文件列表
REMOVE_FILE_LIST=("/usr/share/bash-completion" "/usr/share/doc" "/usr/share/i18n" \
  "/usr/share/info" "/usr/share/licenses" "/usr/share/man" "/usr/share/zoneinfo/Antarctica" \
  "/usr/share/zoneinfo/Arctic" "/usr/share/zoneinfo/[B-D]*" "/usr/share/zoneinfo/E[A-s]*" \
  "/usr/share/zoneinfo/[F-O]*" "/usr/share/zoneinfo/P[A-Z]*" "/usr/share/zoneinfo/P[b-z]*" \
  "/usr/share/zoneinfo/[Q-y]*" "/usr/bin/objdump" "/usr/bin/readelf")

# yum安装所需rpm包
YUM_RPM_LIST=("python3" "python3-dnf" "python3-libdnf" "libdnf" "librepo" "dnf" "systemd" "libsolv" "yum" "sqlite" "gpgme" "libcurl" "libssh" \
  "libpsl" "brotli" "python3-rpm" "python3-libcomps" "python3-gpgme" "python3-hawkey" "util-linux" "fuse")

# debug需要的工具
DEBUG_RPM_LIST=("strace" "audit" "lsof" "tcpdump" "less" "gdb")
DEBUG_PYTHON_LIST=("py-spy")

PYTHON_CMC_BASE_URL=cmc.centralrepo.rnd.huawei.com
PYTHON_CMC_URL=https://${PYTHON_CMC_BASE_URL}/pypi/simple

function add_debug_pkg() {
  if [ ${tag_image} != "debug" ]; then
    return
  fi

  for rpm_name in ${DEBUG_RPM_LIST[*]}; do
    echo "y" | yum install ${rpm_name}
    if [ $? -ne 0 ]; then
      echo "install rpm ${rpm_name} failed"
      return 1
    fi
  done

  pip3 config set global.index-url ${PYTHON_CMC_URL}
  pip3 config set global.trusted-host ${PYTHON_CMC_BASE_URL}
  pip3 config set install.trusted-host ${PYTHON_CMC_BASE_URL}
  pip3 config set download.trusted-host ${PYTHON_CMC_BASE_URL}

  for py_name in ${DEBUG_PYTHON_LIST[*]}; do
    pip3 install ${py_name} --no-cache-dir
    if [ $? -ne 0 ]; then
      echo "pip3 install ${py_name} failed"
      return 1
    fi
  done
}

function add_rpm() {
  cd /mnt
  # yum安装
  for rpm_name in ${YUM_RPM_LIST[*]}; do
    rpm -ivh ${rpm_name}-[0-9]*euleros*.rpm --nodeps --force
    if [ $? -ne 0 ]; then
      echo "install rpm ${rpm_name} failed"
      return 1
    fi
  done
  # rpm包安装
  for rpm_name in ${RPM_EULER_LIST[*]}; do
    echo "y" | yum install ${rpm_name}
    if [ $? -ne 0 ]; then
      echo "install rpm ${rpm_name} failed"
      return 1
    fi
  done
  # 镜像瘦身，清除临时文件
  yum clean all
}

function remove_rpm() {
  for rpm_name in ${REMOVE_RPM_LIST[*]}; do
    rpm -qa | grep ^${rpm_name}-[0-9] | xargs -i rpm -e {} --nodeps
    if [ $? -ne 0 ]; then
      echo "remove rpm ${rpm_name} failed"
      return 1
    fi
  done
}

function remove_file() {
  for file_name in ${REMOVE_FILE_LIST[*]}; do
    rm -rf ${file_name}
    if [ $? -ne 0 ]; then
      echo "remove ${file_name} failed"
      return 1
    fi
  done
}

function main() {
  sed -i "/^baseurl=.*/c\baseurl=${base_euler_yum_url}" /etc/yum.repos.d/slim.repo
  echo "sslverify=0" >>/etc/yum.repos.d/slim.repo
  add_rpm
  if [ $? -ne 0 ]; then
    echo "function add_rpm failed"
    return 1
  fi
  remove_rpm
  if [ $? -ne 0 ]; then
    echo "function remove_rpm failed"
    return 1
  fi
  remove_file
  if [ $? -ne 0 ]; then
    echo "function remove_file failed"
    return 1
  fi
  # 增加dnsmasq用户
  groupadd -g 992 dnsmasq
  useradd -u 992 -g dnsmasq dnsmasq

  add_debug_pkg
}
if [ "$1" == "asan" ]; then
  tag_image=debug
else
  tag_image="$1"
fi
main
