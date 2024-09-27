import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddInstanceComponent } from './add-instance.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AddInstanceComponent],
  imports: [CommonModule, BaseModule]
})
export class AddInstanceModule {}
