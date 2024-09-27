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
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table/pro-table.module';
import { ConfigTableModule } from './config-table/config-table.module';
import { ConfiguringPortsComponent } from './configuring-ports.component';
import { CreatLogicPortModule } from './creat-logic-port/creat-logic-port.module';
import { RouteTableModule } from './route-table/route-table.module';

@NgModule({
  declarations: [ConfiguringPortsComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    CreatLogicPortModule,
    ProButtonModule,
    RouteTableModule,
    ConfigTableModule
  ],
  exports: [ConfiguringPortsComponent]
})
export class ConfiguringPortsModule {}
