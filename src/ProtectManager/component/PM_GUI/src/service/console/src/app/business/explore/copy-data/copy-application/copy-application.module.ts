import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { CopyApplicationRoutingModule } from 'app/business/explore/copy-data/copy-application/copy-application-routing.module';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';
import { CopyApplicationComponent } from 'app/business/explore/copy-data/copy-application/copy-application.component';

@NgModule({
  declarations: [CopyApplicationComponent],
  imports: [CommonModule, CopyApplicationRoutingModule, SubAppCardModule],
  exports: [CopyApplicationComponent]
})
export class CopyApplicationModule {}
