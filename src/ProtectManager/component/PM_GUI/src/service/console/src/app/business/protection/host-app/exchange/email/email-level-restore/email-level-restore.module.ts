import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { EmailLevelRestoreComponent } from './email-level-restore.component';

@NgModule({
  declarations: [EmailLevelRestoreComponent],
  imports: [CommonModule, BaseModule],
  exports: [EmailLevelRestoreComponent]
})
export class EmailLevelRestoreModule {}
