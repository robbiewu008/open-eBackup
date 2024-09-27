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
import { AlertModule } from '@iux/live';

import { HostRegisterRoutingModule } from './host-register-routing.module';
import { HostRegisterComponent } from './host-register.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddHostIngfoComponent } from './add-host-ingfo/add-host-ingfo.component';

@NgModule({
  declarations: [HostRegisterComponent, AddHostIngfoComponent],
  imports: [
    CommonModule,
    HostRegisterRoutingModule,
    BaseModule,
    ProTableModule,
    AlertModule
  ]
})
export class HostRegisterModule {}
