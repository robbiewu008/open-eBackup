#include "afs/PartitionFactory.h"
#include "afs/RawReader.h"
// 分区工厂
PartitionFacotry::PartitionFacotry()
{
    registerClass("mbr", rawReader::CreateObject);
}
