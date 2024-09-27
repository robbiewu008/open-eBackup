import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { DatabaseConfigComponent } from './database-config.component';
import { CustomTableSearchModule } from '../custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [DatabaseConfigComponent],
  imports: [BaseModule, CustomTableSearchModule],
  exports: [DatabaseConfigComponent]
})
export class DatabaseConfigModule {}
