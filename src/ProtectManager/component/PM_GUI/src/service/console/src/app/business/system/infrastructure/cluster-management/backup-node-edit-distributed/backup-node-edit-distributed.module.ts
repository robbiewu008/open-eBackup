import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import {
  ProFilterSearchModule,
  ProTableModule
} from 'app/shared/components/pro-table';
import { BackupNodeEditDistributedComponent } from './backup-node-edit-distributed.component';

@NgModule({
  declarations: [BackupNodeEditDistributedComponent],
  imports: [
    CommonModule,
    BaseModule,
    AlertModule,
    ProFilterSearchModule,
    ProTableModule
  ]
})
export class BackupNodeEditDistributedModule {}
