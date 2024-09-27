import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { SpecifyDestinationLocationModule } from '@backup-policy/specify-destination-location/specify-destination-location.module';
import { BaseModule } from 'app/shared';
import { GeneralAdvancedParameterComponent } from './general-advanced-parameter.component';

@NgModule({
  declarations: [GeneralAdvancedParameterComponent],
  imports: [CommonModule, BaseModule, SpecifyDestinationLocationModule],
  exports: [GeneralAdvancedParameterComponent]
})
export class GeneralAdvancedParameterModule {}
