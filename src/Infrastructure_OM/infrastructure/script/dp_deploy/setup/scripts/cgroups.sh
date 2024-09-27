#!/bin/bash
configPath="/etc/dsware_cgroups_with_ivs"
containerPath="/etc/simbaos_cgroup.conf"

if [ ! -f $configPath ]; then
  touch $configPath
  chmod 644 $configPath
  echo "scene=op" >> $configPath
fi

if [ ! -f $containerPath ]; then
  touch $containerPath
  chmod 640 $containerPath
  echo "SLICE=container.slice" >> $containerPath
fi

sys_group="/sys/fs/cgroup/cpuset/fs8_grp_sys/"
fs_group="/sys/fs/cgroup/cpuset/fs8_grp_fs/"
dsware_group="/sys/fs/cgroup/memory/dsware_mem_cgroup/"

container_group_cpuset="/sys/fs/cgroup/cpuset/container.slice/"
container_group_memory="/sys/fs/cgroup/memory/container.slice/"

base_path="/sys/fs/cgroup/"

if [ ! -e $fs_group ]; then
  mkdir $fs_group
fi
cat $base_path"cpuset/cpuset.mems" > $fs_group"cpuset.mems"
echo "8-23,48-85" > $fs_group"cpuset.cpus"
sed -i "/fs8_grp_fs/d" $configPath
echo "dsware_main_cpu_path=${fs_group}tasks" >> $configPath


if [ ! -e $sys_group ]; then
  mkdir $sys_group
fi
cat $base_path"cpuset/cpuset.mems" > $sys_group"cpuset.mems"
echo "0-7,24-37" > ${sys_group}"cpuset.cpus"
sed -i "/fs8_grp_sys/d" ${configPath}
echo "dsware_other_cpu_path=${sys_group}tasks" >> $configPath
echo "dsware_management_cpu_path=${sys_group}tasks" >> $configPath


if [ ! -e $dsware_group ]; then
  mkdir $dsware_group
fi
echo 192G > $dsware_group"memory.limit_in_bytes"
sed -i "/dsware_mem_cgroup/d" $configPath
echo "dsware_mem_path=${dsware_group}tasks" >> $configPath


if [ ! -e $container_group_cpuset ]; then
  mkdir $container_group_cpuset
fi
cat $base_path"cpuset/cpuset.mems" > $container_group_cpuset"cpuset.mems"
echo "38-47,86-95" > ${container_group_cpuset}"cpuset.cpus"

sed -i "/CPUS/d" $containerPath
echo "CPUS=38-47,86-95" >> $containerPath

sed -i "/SYSTEM_CPUS/d" $containerPath
echo "SYSTEM_CPUS=0-37,48-85" >> $containerPath


if [ ! -e $container_group_memory ]; then
  mkdir $container_group_memory
fi
echo 64G > $container_group_memory"memory.limit_in_bytes"

