import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { VerificateResultComponent } from './verificate-result.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [VerificateResultComponent],
  imports: [CommonModule, BaseModule],
  exports: [VerificateResultComponent]
})
export class VerificateResultModule {}
