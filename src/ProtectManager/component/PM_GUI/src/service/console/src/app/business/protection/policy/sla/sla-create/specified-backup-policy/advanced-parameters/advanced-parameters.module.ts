import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { MultiDuduplicationTipModule } from '@backup-policy/multi-duduplication-tip/multi-duduplication-tip.module';
import { SpecifyDestinationLocationModule } from '@backup-policy/specify-destination-location/specify-destination-location.module';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { AdvancedParametersComponent } from './advanced-parameters.component';

@NgModule({
  declarations: [AdvancedParametersComponent],
  imports: [
    CommonModule,
    BaseModule,
    MultiDuduplicationTipModule,
    SpecifyDestinationLocationModule,
    AlertModule
  ],
  exports: [AdvancedParametersComponent]
})
export class AdvancedParametersModule {}
