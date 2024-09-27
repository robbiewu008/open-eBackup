import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { InitConfigProcessComponent } from './init-config-process.component';
import { BaseModule } from 'app/shared';
import { DownloadLogsModule } from './download-logs/download-logs.module';

@NgModule({
  declarations: [InitConfigProcessComponent],
  imports: [CommonModule, BaseModule, DownloadLogsModule],
  exports: [InitConfigProcessComponent]
})
export class InitConfigProcessModule {}
