import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { FusionSummaryComponent } from './fusion-summary.component';

@NgModule({
  declarations: [FusionSummaryComponent],
  imports: [
    BaseModule,
    BaseInfoModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule
  ]
})
export class FusionSummaryModule {}
