import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SetBlockSizeModule } from './storage-device-detail/set-block-size/set-block-size.module';
import { StorageDeviceDetailModule } from './storage-device-detail/storage-device-detail.module';
import { StorageDeviceListComponent } from './storage-device-list.component';

@NgModule({
  declarations: [StorageDeviceListComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    SetBlockSizeModule,
    StorageDeviceDetailModule
  ],
  exports: [StorageDeviceListComponent]
})
export class StorageDeviceListModule {}
