import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SetFileBlockingComponent } from './set-file-blocking.component';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [SetFileBlockingComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule],
  exports: [SetFileBlockingComponent]
})
export class SetFileBlockingModule {}
