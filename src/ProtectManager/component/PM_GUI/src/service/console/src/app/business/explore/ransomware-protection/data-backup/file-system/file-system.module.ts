import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { FileSystemComponent } from './file-system.component';
import { ManualDetecteModule } from './manual-detecte/manual-detecte.module';

@NgModule({
  declarations: [FileSystemComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ManualDetecteModule
  ],
  exports: [FileSystemComponent]
})
export class FileSystemModule {}
