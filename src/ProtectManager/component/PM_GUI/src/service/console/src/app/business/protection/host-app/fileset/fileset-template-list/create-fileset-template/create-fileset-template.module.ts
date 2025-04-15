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
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { SelectSlaModule } from 'app/shared/components/protect/select-sla/select-sla.module';
import { CreateFilesetModule } from '../../create-fileset/create-fileset.module';
import { CreateFilesetTemplateComponent } from './create-fileset-template.component';
import { TemplateAdvancedParameterModule } from './template-advanced-parameter/template-advanced-parameter.module';
import { BatchResultsModule } from './batch-results/batch-results.module';
import { AdvancedParameterModule } from '../../advanced-parameter/advanced-parameter.module';
@NgModule({
  declarations: [CreateFilesetTemplateComponent],
  imports: [
    CommonModule,
    BaseModule,
    CreateFilesetModule,
    SelectSlaModule,
    TemplateAdvancedParameterModule,
    BatchResultsModule,
    AdvancedParameterModule
  ]
})
export class CreateFilesetTemplateModule {}
