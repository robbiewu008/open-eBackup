import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { CopyBigDataRoutingModule } from './copy-big-data-routing.module';
import { CopyBigDataComponent } from './copy-big-data.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [CopyBigDataComponent],
  imports: [CommonModule, CopyBigDataRoutingModule, SubAppCardModule],
  exports: [CopyBigDataComponent]
})
export class CopyBigDataModule {}
