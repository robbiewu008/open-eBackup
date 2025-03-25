PACKAGE_BASE_PATH="/opt/DataBackup/packages"
SIMBAOS_PACKAGE_PATH="/opt/k8s/SimbaOS/package"
SIMBAOS_USER="ContainerOSUser"
SIMBAOS_GROUP="ContainerOSGroup"
SIMBAOS_SMARTKUBE_INSTALL_PATH="/usr/bin/"


function set_selinux() {
  if [ ! -f '/etc/selinux/config' ]; then
    return 1
  fi
  echo $1

  if [[ "$1" == 'permissive' || "$1" == 'disabled' ]]; then
    sed -i "s/^SELINUX=.*/SELINUX=$1/g" /etc/selinux/config
    return 0
  fi

  echo "Unknown selinux value $1"
  return 1
}
