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

import { FileInterceptionRoutingModule } from './file-interception-routing.module';
import { FileInterceptionComponent } from './file-interception.component';
import { BaseModule } from 'app/shared';
import { FileSystemModule } from './file-system/file-system.module';
import { FilterRuleModule } from './filter-rule/filter-rule.module';

@NgModule({
  declarations: [FileInterceptionComponent],
  imports: [
    CommonModule,
    FileInterceptionRoutingModule,
    BaseModule,
    FileSystemModule,
    FilterRuleModule
  ],
  exports: [FileInterceptionComponent]
})
export class FileInterceptionModule {}
