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
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { NdmpRoutingModule } from './ndmp-routing.module';
import { NdmpComponent } from './ndmp.component';
import { DoradoFileSystemModule } from '../dorado-file-system/dorado-file-system.module';
import { BaseModule } from 'app/shared';
import { DatabaseTemplateModule } from '../../host-app/database-template/database-template.module';
import { CreateNdmpModule } from './create-ndmp/create-ndmp.module';

@NgModule({
  declarations: [NdmpComponent],
  imports: [
    CommonModule,
    NdmpRoutingModule,
    BaseModule,
    DoradoFileSystemModule,
    DatabaseTemplateModule,
    CreateNdmpModule
  ]
})
export class NdmpModule {}
