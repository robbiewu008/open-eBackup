import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProcessLoadingComponent } from './process-loading.component';

@NgModule({
  declarations: [ProcessLoadingComponent],
  imports: [CommonModule, BaseModule],
  exports: [ProcessLoadingComponent]
})
export class ProcessLoadingModule {}
