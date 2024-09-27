import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddStorageDeviceComponent } from './add-storage-device.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AddStorageDeviceComponent],
  imports: [CommonModule, BaseModule]
})
export class AddStorageDeviceModule {}
