import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { CopyBareMetalRoutingModule } from './copy-bare-metal-routing.module';
import { CopyBareMetalComponent } from './copy-bare-metal.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [CopyBareMetalComponent],
  imports: [CommonModule, CopyBareMetalRoutingModule, SubAppCardModule],
  exports: [CopyBareMetalComponent]
})
export class CopyBareMetalModule {}
