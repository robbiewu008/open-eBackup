set -ex

CALL_BASH_DIR=`pwd`
pushd $CALL_BASH_DIR

echo WORKSPACE=$WORKSPACE

DIR1=`pwd`
echo "DIR1=$DIR1"

CURRENT_DIR=$(cd "$(dirname $0)" && pwd)

cd $CURRENT_DIR

sh build_opensource.sh

DIR2=`pwd`
echo "DIR2=$DIR2"

popd

DIR3=`pwd`
echo "DIR3=$DIR3"