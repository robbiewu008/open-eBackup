import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { VirtualizationRoutingModule } from './virtualization-routing.module';
import { VirtualizationComponent } from './virtualization.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [VirtualizationComponent],
  imports: [CommonModule, VirtualizationRoutingModule, SubAppCardModule],
  exports: [VirtualizationComponent]
})
export class VirtualizationModule {}
