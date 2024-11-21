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
import { FilterRuleComponent } from './filter-rule.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { CreateFilterRuleModule } from './create-filter-rule/create-filter-rule.module';
import { AssociateFileSystemModule } from './associate-file-system/associate-file-system.module';
import { FileSystemNumModule } from './file-system-num/file-system-num.module';

@NgModule({
  declarations: [FilterRuleComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    CreateFilterRuleModule,
    AssociateFileSystemModule,
    FileSystemNumModule
  ],
  exports: [FilterRuleComponent]
})
export class FilterRuleModule {}
