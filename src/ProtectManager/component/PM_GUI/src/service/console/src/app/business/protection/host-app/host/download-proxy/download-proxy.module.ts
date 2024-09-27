import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule, GroupModule, RadioModule, SelectModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { DownloadProxyComponent } from './download-proxy.component';
@NgModule({
  declarations: [DownloadProxyComponent],
  imports: [
    CommonModule,
    BaseModule,
    RadioModule,
    SelectModule,
    GroupModule,
    ProTableModule,
    AlertModule
  ],
  exports: [DownloadProxyComponent]
})
export class DownloadProxyModule {}
