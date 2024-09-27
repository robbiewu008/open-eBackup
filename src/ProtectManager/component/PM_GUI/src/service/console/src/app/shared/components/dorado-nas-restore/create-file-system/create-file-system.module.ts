import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared/base.module';
import { CreateFileSystemComponent } from './create-file-system.component';

@NgModule({
  declarations: [CreateFileSystemComponent],
  imports: [CommonModule, BaseModule],
  exports: [CreateFileSystemComponent]
})
export class CreateFileSystemModule {}
