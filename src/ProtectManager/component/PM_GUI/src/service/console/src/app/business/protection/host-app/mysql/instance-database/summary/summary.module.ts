import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SummaryComponent } from './summary.component';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProxyHostModule } from '../../cluster/detail/basic-info/proxy-host/proxy-host.module';

@NgModule({
  declarations: [SummaryComponent],
  imports: [CommonModule, BaseModule, BaseInfoModule, ProxyHostModule],
  exports: [SummaryComponent]
})
export class SummaryModule {}
