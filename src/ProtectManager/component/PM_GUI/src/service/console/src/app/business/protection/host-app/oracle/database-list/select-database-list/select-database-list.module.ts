import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import {
  SelectDatabaseListComponent,
  SelectionPipe
} from './select-database-list.component';

@NgModule({
  declarations: [SelectDatabaseListComponent, SelectionPipe],
  imports: [CommonModule, BaseModule],
  exports: [SelectDatabaseListComponent]
})
export class SelectDatabaseListModule {}
