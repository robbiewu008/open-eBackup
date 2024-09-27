import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SelectionPipe, SelectTableComponent } from './select-table.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [SelectTableComponent, SelectionPipe],
  imports: [CommonModule, BaseModule],
  exports: [SelectTableComponent]
})
export class SelectTableModule {}
