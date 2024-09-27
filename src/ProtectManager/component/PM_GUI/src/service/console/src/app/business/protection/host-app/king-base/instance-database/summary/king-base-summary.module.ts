import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { KingBaseSummaryComponent } from './king-base-summary.component';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { KingBaseProxyHostModule } from './proxy-host/king-base-proxy-host.module';

@NgModule({
  declarations: [KingBaseSummaryComponent],
  imports: [CommonModule, BaseModule, BaseInfoModule, KingBaseProxyHostModule],
  exports: [KingBaseSummaryComponent]
})
export class KingBaseSummaryModule {}
