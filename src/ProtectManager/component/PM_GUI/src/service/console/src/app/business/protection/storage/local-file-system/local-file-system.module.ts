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
import { LocalFileSystemRoutingModule } from './local-file-system-routing.module';
import { LocalFileSystemComponent } from './local-file-system.component';
import { DoradoFileSystemModule } from '../dorado-file-system/dorado-file-system.module';

@NgModule({
  declarations: [LocalFileSystemComponent],
  imports: [CommonModule, LocalFileSystemRoutingModule, DoradoFileSystemModule],
  exports: [LocalFileSystemComponent]
})
export class LocalFileSystemModule {}
