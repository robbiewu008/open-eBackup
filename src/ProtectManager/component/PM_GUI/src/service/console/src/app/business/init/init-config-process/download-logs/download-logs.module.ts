import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DownloadLogsComponent } from './download-logs.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [DownloadLogsComponent],
  imports: [CommonModule, BaseModule, ProTableModule]
})
export class DownloadLogsModule {}
