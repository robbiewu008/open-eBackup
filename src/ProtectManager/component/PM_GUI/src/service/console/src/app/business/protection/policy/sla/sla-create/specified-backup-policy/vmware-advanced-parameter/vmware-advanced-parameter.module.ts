import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { MultiDuduplicationTipModule } from '@backup-policy/multi-duduplication-tip/multi-duduplication-tip.module';
import { SpecifyDestinationLocationModule } from '@backup-policy/specify-destination-location/specify-destination-location.module';
import { BaseModule } from 'app/shared';
import { VmwareAdvancedParameterComponent } from './vmware-advanced-parameter.component';

@NgModule({
  declarations: [VmwareAdvancedParameterComponent],
  imports: [
    CommonModule,
    BaseModule,
    SpecifyDestinationLocationModule,
    MultiDuduplicationTipModule
  ],

  exports: [VmwareAdvancedParameterComponent]
})
export class VmwareAdvancedParameterModule {}
