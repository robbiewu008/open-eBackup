import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddHostComponent } from './add-host.component';

@NgModule({
  declarations: [AddHostComponent],
  imports: [CommonModule, BaseModule]
})
export class AddHostModule {}
