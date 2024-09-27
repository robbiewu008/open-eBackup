import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { KingBaseProxyHostComponent } from './king-base-proxy-host.component';

@NgModule({
  declarations: [KingBaseProxyHostComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule
  ],
  exports: [KingBaseProxyHostComponent]
})
export class KingBaseProxyHostModule {}
