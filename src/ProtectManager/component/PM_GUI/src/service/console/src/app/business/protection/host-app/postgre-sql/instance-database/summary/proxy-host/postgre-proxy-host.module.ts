import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { PostgreProxyHostComponent } from './postgre-proxy-host.component';

@NgModule({
  declarations: [PostgreProxyHostComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule
  ],
  exports: [PostgreProxyHostComponent]
})
export class PostgreProxyHostModule {}
