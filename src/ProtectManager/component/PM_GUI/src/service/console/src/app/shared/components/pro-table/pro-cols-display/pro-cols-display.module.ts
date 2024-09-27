import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import {
  CheckboxModule,
  GroupModule,
  IconModule,
  PopoverModule
} from '@iux/live';
import { ProColsDisplayComponent } from './pro-cols-display.component';

@NgModule({
  declarations: [ProColsDisplayComponent],
  imports: [
    CommonModule,
    FormsModule,
    PopoverModule,
    IconModule,
    GroupModule,
    CheckboxModule
  ],
  exports: [ProColsDisplayComponent]
})
export class ProColsDisplayModule {}
