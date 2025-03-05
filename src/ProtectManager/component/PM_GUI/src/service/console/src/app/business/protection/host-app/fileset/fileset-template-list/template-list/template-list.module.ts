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
import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { TemplateListComponent } from './template-list.component';
import { CreateTemplateModule } from './create-template/create-template.module';
import { TemplateDetailModule } from './template-detail/template-detail.module';
import { AssociatedFilesetModule } from './associated-fileset/associated-fileset.module';
import { ModifyTemplateInfoModule } from './modify-template-info/modify-template-info.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';
import { CustomTableFilterModule } from '../../../../../../shared/components/custom-table-filter/custom-table-filter.module';

@NgModule({
  declarations: [TemplateListComponent],
  imports: [
    CommonModule,
    BaseModule,
    CreateTemplateModule,
    TemplateDetailModule,
    AssociatedFilesetModule,
    ModifyTemplateInfoModule,
    CustomTableSearchModule,
    CustomTableFilterModule
  ],
  exports: [TemplateListComponent]
})
export class TemplateListModule {}
