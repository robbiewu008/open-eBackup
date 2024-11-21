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

import { DetectionModelRoutingModule } from './detection-model-routing.module';
import { BaseModule } from 'app/shared';
import { DetectionModelComponent } from './detection-model.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddModelModule } from './add-model/add-model.module';

@NgModule({
  declarations: [DetectionModelComponent],
  imports: [
    CommonModule,
    DetectionModelRoutingModule,
    BaseModule,
    ProTableModule,
    AddModelModule
  ]
})
export class DetectionModelModule {}
