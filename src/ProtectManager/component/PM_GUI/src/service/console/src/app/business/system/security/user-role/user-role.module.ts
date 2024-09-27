import { EdituserComponent } from './edituser/edituser.component';
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { UserroleRoutingModule } from './user-role-routing.module';
import { UserroleComponent } from './user-role.component';
import { AssociatedusersComponent } from './associatedusers/associatedusers.component';
import { CreateuserComponent } from './createuser/createuser.component';
import { ResetpwdComponent } from './resetpwd/resetpwd.component';
import { UserdetailComponent } from './userdetail/userdetail.component';
import { UnlockComponent } from './unlock/unlock.component';
import { CustomModalOperateModule } from 'app/shared/components';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { SetEmailComponent } from './set-email/set-email.component';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';
@NgModule({
  declarations: [
    UserroleComponent,
    AssociatedusersComponent,
    CreateuserComponent,
    EdituserComponent,
    ResetpwdComponent,
    UserdetailComponent,
    SetEmailComponent,
    UnlockComponent
  ],
  imports: [
    CommonModule,
    UserroleRoutingModule,
    BaseModule,
    CustomModalOperateModule,
    MultiClusterSwitchModule,
    CustomTableSearchModule
  ]
})
export class UserroleModule {}
