import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ModifyRemarksComponent } from './modify-remarks.component';

import { BaseModule } from 'app/shared';
@NgModule({
  imports: [CommonModule, BaseModule],
  exports: [ModifyRemarksComponent],
  declarations: [ModifyRemarksComponent]
})
export class ModifyRemarksModule {}
