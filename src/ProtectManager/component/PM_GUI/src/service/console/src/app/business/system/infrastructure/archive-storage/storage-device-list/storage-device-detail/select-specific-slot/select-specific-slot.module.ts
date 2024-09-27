import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SelectSpecificSlotComponent } from './select-specific-slot.component';

@NgModule({
  declarations: [SelectSpecificSlotComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule],
  exports: [SelectSpecificSlotComponent]
})
export class SelectSpecificSlotModule {}
