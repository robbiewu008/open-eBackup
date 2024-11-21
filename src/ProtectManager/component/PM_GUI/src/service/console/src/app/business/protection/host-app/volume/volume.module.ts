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
import { DatabaseTemplateModule } from '../database-template/database-template.module';
import { VolumeRoutingModule } from './volume-routing.module';
import { VolumeComponent } from './volume.component';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { CopyDataModule } from '../database-template/copy-data/copy-data.module';
import { CreateVolumeModule } from './create-volume/create-volume.module';
import { VolumeAdvancedParameterModule } from './volume-advanced-parameter/volume-advanced-parameter.module';

@NgModule({
  declarations: [VolumeComponent],
  imports: [
    CommonModule,
    VolumeRoutingModule,
    BaseModule,
    DatabaseTemplateModule,
    MultiClusterSwitchModule,
    CreateVolumeModule,
    CopyDataModule,
    VolumeAdvancedParameterModule
  ]
})
export class VolumeModule {}
