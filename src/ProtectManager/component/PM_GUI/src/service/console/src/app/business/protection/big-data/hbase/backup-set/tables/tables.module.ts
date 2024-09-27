import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { TablesComponent } from './tables.component';

@NgModule({
  declarations: [TablesComponent],
  imports: [CommonModule, BaseModule],
  exports: [TablesComponent]
})
export class TablesModule {}
