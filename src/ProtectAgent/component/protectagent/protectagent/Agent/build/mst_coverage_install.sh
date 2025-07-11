#!/bin/sh
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.


if [ "$AGENT_ROOT" = "" ]
then
    echo "Please 'sh env.sh' first."
    exit 0
fi

#add LD_PRELOAD env first

cd ${AGENT_ROOT}/
ls|grep -v "^obj$"|grep -v "^src$"|grep -v "^open_src$"|xargs rm -rf
rm -rf *.docx

rm -rf ${AGENT_ROOT}/../AGENT_PACK_TEMP/*SRC*
mv ${AGENT_ROOT}/../AGENT_PACK_TEMP/* ./
rmdir ${AGENT_ROOT}/../AGENT_PACK_TEMP
tar zxvf OceanStor*
rm OceanStor* -rf

if [ "`ps -ef|grep "rdagent"|wc -l`" = "2" ]
then
  su - root -c "cd $AGENT_ROOT/bin/;echo -e 'y\n' > install_temp_xgqjgcdsigh.txt;./agent_uninstall.sh < install_temp_xgqjgcdsigh.txt;" < \
  `echo -e 'Root@1234\n' > root_password_dwhckicwegciu.txt;echo root_password_dwhckicwegciu.txt`
fi
su - root -c "cd $AGENT_ROOT/bin/;echo -e 'admin\nAdmin@123\nAdmin@123\n\n\n\n' > install_temp_xgqjgcdsigh.txt; \
              ./agent_install.sh < install_temp_xgqjgcdsigh.txt;rm install_temp_xgqjgcdsigh.txt -rf" < \
    `echo -e 'Root@1234\n' > root_password_dwhckicwegciu.txt;echo root_password_dwhckicwegciu.txt`
rm -rf root_password_dwhckicwegciu.txt 

if [ -f "~/gcov_out.so" ]
then
    rm -rf ~/gcov_out.so
fi
mv ${AGENT_ROOT}/obj/gcov_out.so ~/
