import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { FileServiceRoutingModule } from './file-service-routing.module';
import { FileServiceComponent } from './file-service.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [FileServiceComponent],
  imports: [CommonModule, FileServiceRoutingModule, SubAppCardModule],
  exports: [FileServiceComponent]
})
export class FileServiceModule {}
