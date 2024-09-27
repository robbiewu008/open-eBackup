import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { CopyContainerRoutingModule } from './copy-container-routing.module';
import { CopyContainerComponent } from './copy-container.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [CopyContainerComponent],
  imports: [CommonModule, CopyContainerRoutingModule, SubAppCardModule],
  exports: [CopyContainerComponent]
})
export class CopyContainerModule {}
