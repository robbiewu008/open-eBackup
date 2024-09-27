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
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { ConfiguringPortsModule } from './configuring-ports/configuring-ports.module';
import { ManualConfigPortModule } from './configuring-ports/manual-config-port/manual-config-port.module';
import { InitConfigProcessModule } from './init-config-process/init-config-process.module';
import { InitRoutingModule } from './init-routing.module';
import { InitComponent } from './init.component';
import { DistributedInitModule } from './distributed-init/distributed-init.module';
import { DecoupleInitModule } from './decouple-init/decouple-init.module';

@NgModule({
  declarations: [InitComponent],
  imports: [
    CommonModule,
    InitRoutingModule,
    BaseModule,
    InitConfigProcessModule,
    ConfiguringPortsModule,
    ManualConfigPortModule,
    DistributedInitModule,
    DecoupleInitModule,
    AlertModule
  ],
  exports: [InitComponent]
})
export class InitModule {}
