import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DatabaseConfigModule } from 'app/shared/components/database-config/database-config.module';
import { OracleRestoreComponent } from './oracle-restore.component';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [OracleRestoreComponent],
  imports: [
    BaseModule,
    DatabaseConfigModule,
    ProButtonModule,
    ProTableModule,
    AlertModule
  ]
})
export class OracleRestoreModule {}
