import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { LocalFileSystemRoutingModule } from './local-file-system-routing.module';
import { LocalFileSystemComponent } from './local-file-system.component';
import { DoradoFileSystemModule } from '../dorado-file-system/dorado-file-system.module';

@NgModule({
  declarations: [LocalFileSystemComponent],
  imports: [CommonModule, LocalFileSystemRoutingModule, DoradoFileSystemModule],
  exports: [LocalFileSystemComponent]
})
export class LocalFileSystemModule {}
