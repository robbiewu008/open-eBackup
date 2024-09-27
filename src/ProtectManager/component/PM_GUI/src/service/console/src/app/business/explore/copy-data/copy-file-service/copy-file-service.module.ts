import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { CopyFileServiceRoutingModule } from './copy-file-service-routing.module';
import { CopyFileServiceComponent } from './copy-file-service.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [CopyFileServiceComponent],
  imports: [CommonModule, CopyFileServiceRoutingModule, SubAppCardModule],
  exports: [CopyFileServiceComponent]
})
export class CopyFileServiceModule {}
