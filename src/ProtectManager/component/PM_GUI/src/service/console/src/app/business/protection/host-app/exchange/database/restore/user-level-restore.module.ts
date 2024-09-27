import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { TransferModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { UserLevelRestoreComponent } from 'app/business/protection/host-app/exchange/database/restore/user-level-restore.component';

@NgModule({
  declarations: [UserLevelRestoreComponent],
  imports: [CommonModule, BaseModule, TransferModule],
  exports: [UserLevelRestoreComponent]
})
export class UserLevelRestoreModule {}
