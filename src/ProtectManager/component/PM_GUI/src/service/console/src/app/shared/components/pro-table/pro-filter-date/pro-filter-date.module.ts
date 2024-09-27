import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ProFilterDateComponent } from './pro-filter-date.component';
import {
  ButtonModule,
  DatePickerModule,
  GroupModule,
  IconModule,
  PopoverModule
} from '@iux/live';
import { FormsModule } from '@angular/forms';

@NgModule({
  declarations: [ProFilterDateComponent],
  imports: [
    CommonModule,
    FormsModule,
    PopoverModule,
    IconModule,
    ButtonModule,
    GroupModule,
    DatePickerModule
  ],
  exports: [ProFilterDateComponent]
})
export class ProFilterDateModule {}
