import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyDataListModule } from 'app/shared/components/copy-data-list/copy-data-list.module';
import { FileDetailComponent } from './file-detail.component';

@NgModule({
  declarations: [FileDetailComponent],
  imports: [CommonModule, BaseModule, CopyDataListModule]
})
export class FileDetailModule {}
