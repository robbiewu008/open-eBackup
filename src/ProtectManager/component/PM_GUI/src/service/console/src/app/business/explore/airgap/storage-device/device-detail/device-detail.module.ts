import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DeviceDetailComponent } from './device-detail.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [DeviceDetailComponent],
  imports: [CommonModule, BaseModule],
  exports: [DeviceDetailComponent]
})
export class DeviceDetailModule {}
