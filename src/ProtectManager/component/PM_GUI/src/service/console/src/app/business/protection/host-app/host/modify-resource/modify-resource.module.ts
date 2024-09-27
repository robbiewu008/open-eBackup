import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ModifyResourceComponent } from './modify-resource.component';

@NgModule({
  declarations: [ModifyResourceComponent],
  imports: [CommonModule, BaseModule],
  exports: [ModifyResourceComponent]
})
export class ModifyResourceModule {}
