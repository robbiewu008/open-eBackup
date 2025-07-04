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

import { CnwareRoutingModule } from './cnware-routing.module';
import { CnwareComponent } from './cnware.component';
import { VirtualizationBaseModule } from '../virtualization-base/virtualization-base.module';
import { SummaryComponent } from './summary/summary.component';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { CopyDataComponent } from './copy-data/copy-data.component';
import { CopyDataModule as CommonCopyDataModule } from 'app/shared/components/copy-data/copy-data.module';
import { SpecialBaseInfoModule } from 'app/shared/components/special-base-info/special-base-info.module';
import { CustomModalOperateModule } from 'app/shared/components/custom-modal-operate/custom-modal-operate.module';
import { BaseTableModule } from '../virtualization-base/base-table/base-table.module';

@NgModule({
  declarations: [CnwareComponent, SummaryComponent, CopyDataComponent],
  imports: [
    CommonModule,
    CnwareRoutingModule,
    BaseModule,
    VirtualizationBaseModule,
    BaseTableModule,
    BaseInfoModule,
    SpecialBaseInfoModule,
    CustomModalOperateModule,
    ProTableModule,
    CommonCopyDataModule
  ]
})
export class CnwareModule {}
