import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { PostgreSummaryComponent } from './postgre-summary.component';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { PostgreProxyHostModule } from './proxy-host/postgre-proxy-host.module';

@NgModule({
  declarations: [PostgreSummaryComponent],
  imports: [CommonModule, BaseModule, BaseInfoModule, PostgreProxyHostModule],
  exports: [PostgreSummaryComponent]
})
export class PostgreSummaryModule {}
