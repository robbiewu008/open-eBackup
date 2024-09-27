import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ModifyAsmComponent } from './modify-asm.component';

@NgModule({
  declarations: [ModifyAsmComponent],
  imports: [CommonModule, BaseModule],
  exports: [ModifyAsmComponent]
})
export class ModifyAsmModule {}
