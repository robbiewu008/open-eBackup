CURRENT_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
HOME=${CURRENT_PATH}/../..
export AGENT_ROOT=${HOME}/Agent
export PATH=.:${PATH}:${AGENT_ROOT}/bin
# Debug or Release
export BUILD_TYPE=Release

if [ -z ${LD_LIBRARY_PATH} ]; then
    export LD_LIBRARY_PATH=${AGENT_ROOT}/bin
else
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${AGENT_ROOT}/bin
fi
sys=`uname -s`

STDCPP_LIB_PATH=""
GCC_S_LIB_PATH=""
SSP_LIB_PATH=""
if [ "${sys}" = "AIX" ]; then
    version=`oslevel | grep 6`
    if [ $? -eq 0 -o -n "${version}" ]; then
        export AIX_STDCPP_PATH="/opt/freeware/lib/gcc/powerpc-ibm-aix6.1.0.0/8.3.0/pthread/libstdc++.a"
        export AIX_GCC_S_PATH="/opt/freeware/lib/gcc/powerpc-ibm-aix6.1.0.0/8.3.0/pthread/libgcc_s.a"
    else
        export AIX_STDCPP_PATH="/opt/freeware/lib/gcc/powerpc-ibm-aix7.1.0.0/8.3.0/pthread/libstdc++.a"
        export AIX_GCC_S_PATH="/opt/freeware/lib/gcc/powerpc-ibm-aix7.1.0.0/8.3.0/pthread/libgcc_s.a"
    fi
    # cmake compile [ON/OFF]
    export BUILD_CMAKE=ON
elif [ "${sys}" = "SunOS" ]; then
    export BUILD_CMAKE=ON
    export LD_LIBRARY_PATH_64=/usr/local/gcc-7.3.0/lib:/usr/local/gcc-7.3.0/lib/sparcv9:/usr/local/isl-0.15/lib:/usr/local/mpc-1.0.3/lib:/usr/local/mpfr-3.1.4/lib:/usr/local/gmp-6.1.0/lib:/usr/local/gmp-6.1.0/lib:/usr/sfw/lib/sparcv9:/usr/lib/sparcv9
    export PATH=/usr/local/cmake-3.26.3/bin:/usr/sfw/sparc-sun-solaris2.10/bin:/usr/local/gcc-7.3.0/bin:/tools/jdk/bin:/usr/sfw/bin:/usr/sbin:/usr/bin:/usr/sbin:/sbin:/usr/bin:/usr/local/bin:/usr/ccs/bin:/usr/ucb:/usr/perl5/5.8.4/bin:${PATH}
    export STDCPP_LIB_PATH="/usr/local/gcc-7.3.0/lib/sparcv9/libstdc++.so.6"
    export GCC_S_LIB_PATH="/usr/local/gcc-7.3.0/lib/sparcv9/libgcc_s.so.1"
    export SSP_LIB_PATH="/usr/local/gcc-7.3.0/lib/sparcv9/libssp.so.0"
    export JAVA_HOME=/tools/jdk
else
    export BUILD_CMAKE=OFF
fi

export TERM=vt100

PS1="\h [\u]:\w # "
alias h="history"
alias ll="ls -l"
alias lsa="ls -al"
alias dir="ls -lF"
SH=
sys=`uname -s`
if [ "$sys" = "SunOS" ]; then
   SH=bash
else
   SH=sh
fi

alias mk="$SH ${AGENT_ROOT}/build/agent_make.sh $*"
alias mkd="$SH ${AGENT_ROOT}/build/driver/agent_drv_make.sh $*"
alias pk="$SH ${AGENT_ROOT}/build/agent_pack.sh $*"
alias pkom="$SH ${AGENT_ROOT}/build/agent_pack_mobility.sh $*"
alias bin="cd ${AGENT_ROOT}/bin"
alias conf="cd ${AGENT_ROOT}/conf"
alias obj="cd ${AGENT_ROOT}/obj"
alias src="cd ${AGENT_ROOT}/src/src"
alias inc="cd ${AGENT_ROOT}/src/inc"
alias 3src="cd ${AGENT_ROOT}/open_src"
alias test="cd ${AGENT_ROOT}/test"
alias build="cd ${AGENT_ROOT}/build"
alias log="cd ${AGENT_ROOT}/log"
alias tmp="cd ${AGENT_ROOT}/tmp"
alias pp="ps -ef | grep rdagent; ps -ef |grep nginx"
alias ppp="ps -ef | grep -v grep | grep rdagent | grep ${LOGNAME}; ps -ef | grep -v grep | grep nginx | grep ${LOGNAME}"
alias stop="${AGENT_ROOT}/bin/agent_stop.sh"
alias start="${AGENT_ROOT}/bin/agent_start.sh"

chmod 0755 ${AGENT_ROOT}/build/*.sh

