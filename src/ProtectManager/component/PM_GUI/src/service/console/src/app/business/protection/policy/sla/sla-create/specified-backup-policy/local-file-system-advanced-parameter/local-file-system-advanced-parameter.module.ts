import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LocalFileSystemAdvancedParameterComponent } from './local-file-system-advanced-parameter.component';

@NgModule({
  declarations: [LocalFileSystemAdvancedParameterComponent],
  imports: [CommonModule, BaseModule],
  exports: [LocalFileSystemAdvancedParameterComponent]
})
export class LocalFileSystemAdvancedParameterModule {}
