import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { UploadModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { ImportRevocationListComponent } from './import-revocation-list.component';

@NgModule({
  declarations: [ImportRevocationListComponent],
  imports: [CommonModule, BaseModule, UploadModule],
  exports: [ImportRevocationListComponent]
})
export class ImportRevocationListModule {}
