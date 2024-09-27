import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table/pro-table.module';
import { BackupStorageDeviceModule } from '../../backup-storage-device/backup-storage-device.module';
import { UserAuthModule } from '../../user-auth/user-auth.module';
import { BackupStorageUnitDetailComponent } from './backup-storage-unit-detail.component';

@NgModule({
  declarations: [BackupStorageUnitDetailComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    BackupStorageDeviceModule,
    UserAuthModule
  ],
  exports: [BackupStorageUnitDetailComponent]
})
export class BackupStorageUnitDetailModule {}
