import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SelectApplicationComponent } from './select-application.component';

@NgModule({
  declarations: [SelectApplicationComponent],
  imports: [CommonModule, BaseModule],
  exports: [SelectApplicationComponent]
})
export class SelectApplicationModule {}
