import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { FileDetailComponent } from './file-detail.component';

@NgModule({
  declarations: [FileDetailComponent],
  imports: [CommonModule, BaseModule, ProStatusModule, ProTableModule],
  exports: [FileDetailComponent]
})
export class FileDetailModule {}
