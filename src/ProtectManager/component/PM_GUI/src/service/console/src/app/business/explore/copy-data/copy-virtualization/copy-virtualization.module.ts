import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { CopyVirtualizationRoutingModule } from './copy-virtualization-routing.module';
import { CopyVirtualizationComponent } from './copy-virtualization.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [CopyVirtualizationComponent],
  imports: [CommonModule, CopyVirtualizationRoutingModule, SubAppCardModule],
  exports: [CopyVirtualizationComponent]
})
export class CopyVirtualizationModule {}
