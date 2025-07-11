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
import { BaseTemplateComponent } from './base-template.component';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { SummaryModule } from './summary/summary.module';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { CopyDataModule } from './copy-data/copy-data.module';
import { AddResourceTagModule } from 'app/shared/components/add-resource-tag/add-resource-tag.module';

@NgModule({
  declarations: [BaseTemplateComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProButtonModule,
    ProTableModule,
    ProStatusModule,
    SummaryModule,
    BaseInfoModule,
    CopyDataModule,
    AddResourceTagModule
  ],
  exports: [BaseTemplateComponent]
})
export class BaseTemplateModule {}
