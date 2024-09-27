import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { CopyDatabaseRoutingModule } from './copy-database-routing.module';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';
import { CopyDatabaseComponent } from './copy-database.component';

@NgModule({
  declarations: [CopyDatabaseComponent],
  imports: [CommonModule, CopyDatabaseRoutingModule, SubAppCardModule],
  exports: [CopyDatabaseComponent]
})
export class CopyDatabaseModule {}
