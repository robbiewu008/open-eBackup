import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddDetectionModelComponent } from './add-detection-model.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AddDetectionModelComponent],
  imports: [CommonModule, BaseModule],
  exports: [AddDetectionModelComponent]
})
export class AddDetectionModelModule {}
