import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ModifyComponent } from './modify.component';

@NgModule({
  declarations: [ModifyComponent],
  imports: [CommonModule, BaseModule],

  exports: [ModifyComponent]
})
export class ModifyModule {}
