import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FileInterceptionComponent } from './file-interception.component';
import { BaseModule } from 'app/shared';

@NgModule({
  imports: [CommonModule, BaseModule],
  declarations: [FileInterceptionComponent],
  exports: [FileInterceptionComponent]
})
export class FileInterceptionModule {}
