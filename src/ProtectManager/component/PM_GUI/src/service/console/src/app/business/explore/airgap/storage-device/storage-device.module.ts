import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { StorageDeviceComponent } from './storage-device.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { DeviceDetailModule } from './device-detail/device-detail.module';
import { GetTacticsComponent } from './get-tactics/get-tactics.component';
import { GetTacticsModule } from './get-tactics/get-tactics.module';
import { ProButtonModule } from 'app/shared/components/pro-button';

@NgModule({
  declarations: [StorageDeviceComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    GetTacticsModule,
    DeviceDetailModule
  ],
  exports: [StorageDeviceComponent]
})
export class StorageDeviceModule {}
