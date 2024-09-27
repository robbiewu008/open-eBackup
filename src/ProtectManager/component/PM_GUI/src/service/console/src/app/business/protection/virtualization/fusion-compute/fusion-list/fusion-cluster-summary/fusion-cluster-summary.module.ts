import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SpecialBaseInfoModule } from 'app/shared/components/special-base-info/special-base-info.module';
import { FusionClusterSummaryComponent } from './fusion-cluster-summary.component';

@NgModule({
  declarations: [FusionClusterSummaryComponent],
  imports: [
    BaseModule,
    SpecialBaseInfoModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule
  ]
})
export class FusionClusterSummaryModule {}
