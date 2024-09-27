import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { StartDensensitizationComponent } from './start-densensitization.component';
import { AnonymizationVerificateModule } from './anonymization-verificate/anonymization-verificate.module';
import { VerificateResultModule } from './verificate-result/verificate-result.module';
import { ModifyIdentificationResultModule } from './modify-identification-result/modify-identification-result.module';

@NgModule({
  declarations: [StartDensensitizationComponent],
  imports: [
    CommonModule,
    BaseModule,
    AnonymizationVerificateModule,
    VerificateResultModule,
    ModifyIdentificationResultModule
  ],
  exports: [StartDensensitizationComponent]
})
export class StartDensensitizationModule {}
