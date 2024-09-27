export AGENT_ROOT=${HOME}/Agent
export HLT_ROOT=${AGENT_ROOT}/test/stubTest/hlt
export PATH=.:${PATH}
if [ -z ${LD_LIBRARY_PATH} ]
then
    export LD_LIBRARY_PATH=${AGENT_ROOT}/bin
else
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${AGENT_ROOT}/bin
fi

PS1="\h [\u]:\w # "
alias h="history"
alias ll="ls -l"
alias lsa="ls -al"
alias dir="ls -lF"

alias mk="sh ${AGENT_ROOT}/test/stubTest/hlt/build/hlt_make.sh $*"

chmod 0755 ${AGENT_ROOT}/test/stubTest/hlt/build/*.sh

