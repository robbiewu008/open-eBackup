import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { DatabaseRoutingModule } from './database-routing.module';
import { DatabaseComponent } from './database.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [DatabaseComponent],
  imports: [CommonModule, DatabaseRoutingModule, SubAppCardModule],
  exports: [DatabaseComponent]
})
export class DatabaseModule {}
