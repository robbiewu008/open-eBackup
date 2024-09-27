import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SpecifyDestinationLocationComponent } from './specify-destination-location.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [SpecifyDestinationLocationComponent],
  imports: [CommonModule, BaseModule],
  exports: [SpecifyDestinationLocationComponent]
})
export class SpecifyDestinationLocationModule {}
