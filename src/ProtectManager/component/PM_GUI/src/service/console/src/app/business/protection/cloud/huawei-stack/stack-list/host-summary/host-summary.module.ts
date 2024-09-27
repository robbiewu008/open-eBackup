import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { HCSHostSummaryComponent } from './host-summary.component';

@NgModule({
  declarations: [HCSHostSummaryComponent],
  imports: [
    BaseModule,
    BaseInfoModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule
  ]
})
export class HCSHostSummaryModule {}
