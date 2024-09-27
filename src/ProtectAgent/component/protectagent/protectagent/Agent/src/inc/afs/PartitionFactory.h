#ifndef PARTITIONFACTORY_H
#define PARTITIONFACTORY_H

#include "afs/PartitionHandler.h"
#include "afs/MBRHandler.h"
#include "afs/GPTHandler.h"

class PartitionFacotry : public afsObjectFacotry {
public:
    PartitionFacotry();
};
#endif
