import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FileIndexedComponent } from './file-indexed.component';
import { IconModule, TooltipModule } from '@iux/live';

@NgModule({
  declarations: [FileIndexedComponent],
  imports: [CommonModule, IconModule, TooltipModule],
  exports: [FileIndexedComponent]
})
export class FileIndexedModule {}
