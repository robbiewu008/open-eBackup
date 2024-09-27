import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { CopyDataScnSelectComponent } from './copy-data-scn-select.component';

@NgModule({
  declarations: [CopyDataScnSelectComponent],
  imports: [CommonModule, BaseModule],

  exports: [CopyDataScnSelectComponent]
})
export class CopyDataScnSelectModule {}
