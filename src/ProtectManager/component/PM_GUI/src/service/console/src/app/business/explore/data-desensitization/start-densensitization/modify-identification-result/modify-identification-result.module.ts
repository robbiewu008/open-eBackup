import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ModifyIdentificationResultComponent } from './modify-identification-result.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [ModifyIdentificationResultComponent],
  imports: [CommonModule, BaseModule],
  exports: [ModifyIdentificationResultComponent]
})
export class ModifyIdentificationResultModule {}
