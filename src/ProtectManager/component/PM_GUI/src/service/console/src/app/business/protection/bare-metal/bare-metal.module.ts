import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { BareMetalRoutingModule } from './bare-metal-routing.module';
import { BareMetalComponent } from './bare-metal.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [BareMetalComponent],
  imports: [CommonModule, BareMetalRoutingModule, SubAppCardModule],
  exports: [BareMetalComponent]
})
export class BareMetalModule {}
