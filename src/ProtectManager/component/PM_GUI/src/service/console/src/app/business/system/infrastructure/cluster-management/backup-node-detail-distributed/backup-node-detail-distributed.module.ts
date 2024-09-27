import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import {
  ProFilterSearchModule,
  ProTableModule
} from 'app/shared/components/pro-table';
import { StorageDeviceModule } from '../../../../explore/airgap/storage-device/storage-device.module';
import { BackupNodeDetailDistributedComponent } from './backup-node-detail-distributed.component';

@NgModule({
  declarations: [BackupNodeDetailDistributedComponent],
  imports: [
    CommonModule,
    BaseModule,
    AlertModule,
    ProFilterSearchModule,
    ProTableModule,
    StorageDeviceModule
  ]
})
export class BackupNodeDetailDistributedModule {}
