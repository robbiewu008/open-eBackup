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
