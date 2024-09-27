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

