import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { SlaServiceModule } from 'app/shared/services/sla.service';
import { SlaRouterModule } from './sla-router.module';
import { SlaComponent } from './sla.component';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  imports: [
    BaseModule,
    SlaRouterModule,
    SlaServiceModule,
    MultiClusterSwitchModule,
    CustomTableSearchModule
  ],
  declarations: [SlaComponent]
})
export class SlaModule {}
