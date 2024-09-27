import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { BigDataRoutingModule } from './big-data-routing.module';
import { BigDataComponent } from './big-data.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [BigDataComponent],
  imports: [CommonModule, BigDataRoutingModule, SubAppCardModule],
  exports: [BigDataComponent]
})
export class BigDataModule {}
