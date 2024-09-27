import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BigScreenComponent } from './big-screen.component';
import { BaseModule } from 'app/shared';

@NgModule({
  imports: [CommonModule, BaseModule],
  declarations: [BigScreenComponent],
  exports: [BigScreenComponent]
})
export class BigScreenModule {}
