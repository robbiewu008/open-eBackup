import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BackupClusterDistributedComponent } from './backup-cluster-distributed.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import {
  OperationmenuModule,
  SearchModule,
  SortModule,
  TabsModule
} from '@iux/live';
import { StatusModule } from 'app/shared/components';

@NgModule({
  declarations: [BackupClusterDistributedComponent],
  imports: [
    CommonModule,
    BaseModule,
    OperationmenuModule,
    SearchModule,
    SortModule,
    StatusModule,
    TabsModule,
    ProButtonModule,
    ProTableModule
  ]
})
export class BackupClusterDistributedModule {}
