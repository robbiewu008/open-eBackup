import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ModifyAzComponent } from './modify-az.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [ModifyAzComponent],
  imports: [CommonModule, BaseModule]
})
export class ModifyAzModule {}
