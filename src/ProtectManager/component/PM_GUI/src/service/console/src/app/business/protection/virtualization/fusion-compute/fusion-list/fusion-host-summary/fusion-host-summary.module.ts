import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SpecialBaseInfoModule } from 'app/shared/components/special-base-info/special-base-info.module';
import { FusionHostSummaryComponent } from './fusion-host-summary.component';

@NgModule({
  declarations: [FusionHostSummaryComponent],
  imports: [
    BaseModule,
    SpecialBaseInfoModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule
  ]
})
export class FusionHostSummaryModule {}
