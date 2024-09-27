import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SelectCopyResourceComponent } from './select-copy-resource.component';
import { BaseModule } from 'app/shared';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [SelectCopyResourceComponent],
  imports: [CommonModule, BaseModule, AlertModule],
  exports: [SelectCopyResourceComponent]
})
export class SelectCopyResourceModule {}
