import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { KingBaseBasicInfoComponent } from './king-base-basic-info.component';
import { KingBaseProxyHostModule } from './proxy-host/king-base-proxy-host.module';

@NgModule({
  declarations: [KingBaseBasicInfoComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    KingBaseProxyHostModule
  ],
  exports: [KingBaseBasicInfoComponent]
})
export class KingBaseBasicInfoModule {}
