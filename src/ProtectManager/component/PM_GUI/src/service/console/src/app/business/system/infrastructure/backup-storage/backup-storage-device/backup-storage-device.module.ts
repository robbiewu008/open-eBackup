import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { BackupStorageDeviceComponent } from './backup-storage-device.component';

@NgModule({
  declarations: [BackupStorageDeviceComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule],
  exports: [BackupStorageDeviceComponent]
})
export class BackupStorageDeviceModule {}
