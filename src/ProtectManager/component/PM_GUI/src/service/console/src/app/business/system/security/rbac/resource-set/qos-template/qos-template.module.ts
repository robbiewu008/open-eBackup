import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { QosTemplateComponent } from './qos-template.component';

@NgModule({
  declarations: [QosTemplateComponent],
  imports: [
    CommonModule,
    BaseModule,
    MultiClusterSwitchModule,
    CustomTableSearchModule
  ],
  exports: [QosTemplateComponent]
})
export class QosTemplateModule {}
