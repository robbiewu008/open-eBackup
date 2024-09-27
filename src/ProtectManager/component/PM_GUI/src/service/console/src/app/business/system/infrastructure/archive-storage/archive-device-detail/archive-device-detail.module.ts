import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ArchiveDeviceDetailComponent } from './archive-device-detail.component';

@NgModule({
  declarations: [ArchiveDeviceDetailComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [ArchiveDeviceDetailComponent]
})
export class ArchiveDeviceDetailModule {}
