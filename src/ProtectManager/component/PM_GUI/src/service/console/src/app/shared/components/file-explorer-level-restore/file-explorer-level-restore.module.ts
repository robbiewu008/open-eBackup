import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { ObjectRestoreModule } from 'app/business/protection/storage/object/object-service/object-restore/object-restore.module';
import { BaseModule } from 'app/shared/base.module';
import { FileExplorerLevelRestoreComponent } from './file-explorer-level-restore.component';

@NgModule({
  declarations: [FileExplorerLevelRestoreComponent],
  imports: [CommonModule, BaseModule, ObjectRestoreModule],
  exports: [FileExplorerLevelRestoreComponent]
})
export class FileExplorerLevelRestoreModule {}
