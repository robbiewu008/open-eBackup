import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ModifyHandleComponent } from './modify-handle.component';

import { BaseModule } from 'app/shared';
import { AlertModule } from '@iux/live';
@NgModule({
  imports: [CommonModule, BaseModule, AlertModule],
  exports: [ModifyHandleComponent],
  declarations: [ModifyHandleComponent]
})
export class ModifyHandleModule {}
