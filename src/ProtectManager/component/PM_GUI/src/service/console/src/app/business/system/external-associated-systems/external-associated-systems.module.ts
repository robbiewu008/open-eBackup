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

import { ExternalAssociatedSystemsRoutingModule } from './external-associated-systems-routing.module';
import { ExternalAssociatedSystemsComponent } from './external-associated-systems.component';
import { BaseModule } from '../../../shared';
import {
  ButtonModule,
  DatatableModule,
  GroupModule,
  IconModule,
  OperationmenuModule,
  OverflowModule,
  PaginatorModule,
  PopoverModule,
  SearchModule,
  SortModule,
  TabsModule
} from '@iux/live';
import { MultiClusterSwitchModule } from '../../../shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { StatusModule } from '../../../shared/components';
import { ProButtonModule } from '../../../shared/components/pro-button';
import { ProTableModule } from '../../../shared/components/pro-table';
import { CreateExternalSystemModule } from './create-external-system/create-external-system.module';

@NgModule({
  declarations: [ExternalAssociatedSystemsComponent],
  imports: [
    CommonModule,
    ExternalAssociatedSystemsRoutingModule,
    BaseModule,
    ButtonModule,
    DatatableModule,
    GroupModule,
    IconModule,
    MultiClusterSwitchModule,
    OperationmenuModule,
    OverflowModule,
    PaginatorModule,
    PopoverModule,
    SearchModule,
    SortModule,
    StatusModule,
    TabsModule,
    ProButtonModule,
    ProTableModule,
    CreateExternalSystemModule
  ]
})
export class ExternalAssociatedSystemsModule {}
