import { NgModule } from '@angular/core';
import { AddUserComponent } from './add-user.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AddUserComponent],
  imports: [BaseModule]
})
export class AddUserModule {}
