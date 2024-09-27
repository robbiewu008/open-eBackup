import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ModifyHostComponent } from './modify-host.component';

@NgModule({
  declarations: [ModifyHostComponent],
  imports: [CommonModule, BaseModule],
  exports: [ModifyHostComponent]
})
export class ModifyHostModule {}
