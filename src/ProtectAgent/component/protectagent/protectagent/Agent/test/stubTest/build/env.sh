export AGENT_ROOT=${HOME}/Agent
export PATH=.:${PATH} #:${AGENT_ROOT}/bin
if [ -z ${LD_LIBRARY_PATH} ]
then
    export LD_LIBRARY_PATH=${AGENT_ROOT}/bin:${AGENT_ROOT}/open_src/gperftools/.libs:${AGENT_ROOT}/open_src/libevent/.libs
else
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${AGENT_ROOT}/bin:${AGENT_ROOT}/open_src/gperftools/.libs:${AGENT_ROOT}/open_src/libevent/.libs
fi

export TERM=vt100

PS1="\h [\u]:\w # "
alias h="history"
alias ll="ls -l"
alias lsa="ls -al"
alias dir="ls -lF"

alias mk="sh ${AGENT_ROOT}/test/stubTest/build/test_make.sh $*"

chmod 0755 ${AGENT_ROOT}/test/stubTest/build/*.sh

