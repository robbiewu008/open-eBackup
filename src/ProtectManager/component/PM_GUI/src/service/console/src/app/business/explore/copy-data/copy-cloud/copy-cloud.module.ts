import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { CopyCloudRoutingModule } from './copy-cloud-routing.module';
import { CopyCloudComponent } from './copy-cloud.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [CopyCloudComponent],
  imports: [CommonModule, CopyCloudRoutingModule, SubAppCardModule],
  exports: [CopyCloudComponent]
})
export class CopyCloudModule {}
