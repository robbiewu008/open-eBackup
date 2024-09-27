import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { ContainerRoutingModule } from './container-routing.module';
import { ContainerComponent } from './container.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [ContainerComponent],
  imports: [CommonModule, ContainerRoutingModule, SubAppCardModule],
  exports: [ContainerComponent]
})
export class ContainerModule {}
