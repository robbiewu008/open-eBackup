import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { CloudRoutingModule } from './cloud-routing.module';
import { CloudComponent } from './cloud.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [CloudComponent],
  imports: [CommonModule, CloudRoutingModule, SubAppCardModule],
  exports: [CloudComponent]
})
export class CloudModule {}
