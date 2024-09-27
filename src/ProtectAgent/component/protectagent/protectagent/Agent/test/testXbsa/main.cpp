/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "MultiThreadBus.h"
#include <unistd.h>
#include "CommonDefine.h"
#include <vector>

// void fun(int num)
// {
//   while(true)
//   {
//     std::cout << "process " << num << std::endl;
//     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//   }
// }

int main(int argc, char **argv)
{
    if (argc < 2)
    {
      Log("Please input library name.");
      return 0;
    }

    int parell = 0;
    char choice;
    std::vector<int> businessTypeVec;
    std::string libPath("/usr/openv/lib/libxbsa64.so");
    MultiThreadBus bus;

    while((choice = getopt(argc, argv, "b:n:hl:f:")) != EOF)
    {
      switch(choice)
      {
        case 'b':
          businessTypeVec.push_back(atoi(optarg));
          break;
        case 'n':
          parell = atoi(optarg);
          break;
        case 'h':
        {
          std::cout << "options: \n" << 
                       "  -b: backup(0)/recover(1)/delete(2)\n" << 
                       "  -n: concurrency number\n" << 
                       "  -f: file used to backup/recover/delete\n" << 
                       "  -l: libxbsa.so path.default:/usr/openv/lib/libxbsa64.so\n";
          break;
        }
        case 'l':
        {
          libPath = std::string(optarg);
          break;
        }
        case 'f':
        {
          bus.setTestFileName(std::string(optarg));
          break;
        }
        default:
          return 1;
      }
    }

    for (int i = 0; i<businessTypeVec.size(); i++)
    {
      switch(businessTypeVec[i])
      {
        case 0:
        {
          Log("Exec backup %d.", parell);
          bus.startBackupBus(parell, libPath);
          break;
        }
        case 1:
        {
          Log("Exec recover %d.", parell);
          bus.startRecoverBus(parell, libPath);
          break;
        }
        case 2:
        {
          Log("Exec delete %d.", parell);
          bus.startDeleteBus(parell, libPath);
          break;
        }
        default:
          break;
      }
    }   
    return 0;
}

