import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AnonymizationVerificateComponent } from './anonymization-verificate.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AnonymizationVerificateComponent],
  imports: [CommonModule, BaseModule],
  exports: [AnonymizationVerificateComponent]
})
export class AnonymizationVerificateModule {}
