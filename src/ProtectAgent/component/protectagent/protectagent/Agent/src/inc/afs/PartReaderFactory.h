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
#ifndef IMAGE_PARTREADERFACTORY_H_
#define IMAGE_PARTREADERFACTORY_H_
#include <list>
#include "afs/ImgReader.h"

class partReaderFactory {
public:
    partReaderFactory() {}
    ~partReaderFactory();

    imgReader *createPartReaderOBJ(imgReader *imgobj, struct partition *ppart, void *part_other_mode);

private:
    // 空间管理
    list<imgReader *> m_real_part_reader_list;

    void addPartReader(imgReader *imgobj);
};

#endif /* IMAGE_PARTREADERFACTORY_H_ */
