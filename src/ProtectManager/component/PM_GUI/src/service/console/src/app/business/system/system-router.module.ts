import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { CanDeactivateGuard } from 'app/shared/guards/deactivate.guard';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';
import { SystemComponent } from './system.component';

const routes: Routes = [
  {
    path: '',
    component: SystemComponent,
    children: [
      {
        path: '',
        redirectTo: 'infrastructure/cluster-management',
        pathMatch: 'full'
      },
      {
        path: 'infrastructure/cluster-management',
        loadChildren: () =>
          import(
            './infrastructure/cluster-management/cluster-management.module'
          ).then(mod => mod.ClusterManagementModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'infrastructure/local-storage',
        loadChildren: () =>
          import('./infrastructure/local-storage/local-storage.module').then(
            mod => mod.LocalStorageModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'infrastructure/external-storage',
        loadChildren: () =>
          import(
            './infrastructure/external-storage/external-storage.module'
          ).then(mod => mod.ExternalStorageModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'infrastructure/archive-storage',
        loadChildren: () =>
          import(
            './infrastructure/archive-storage/archive-storage.module'
          ).then(mod => mod.ArchiveStorageModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'infrastructure/backup-storage',
        loadChildren: () =>
          import(
            './infrastructure/archive-storage/archive-storage.module'
          ).then(mod => mod.ArchiveStorageModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'infrastructure/nas-backup-storage',
        loadChildren: () =>
          import(
            './infrastructure/backup-storage/distributed-nas-list.module'
          ).then(mod => mod.DistributedNasListModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'infrastructure/hcs-storage',
        loadChildren: () =>
          import('./infrastructure/hcs-storage/hcs-storage.module').then(
            mod => mod.HcsStorageModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'security/rbac',
        loadChildren: () =>
          import('./security/rbac/rbac.module').then(mod => mod.RbacModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'security/userrole',
        loadChildren: () =>
          import('./security/user-role/user-role.module').then(
            mod => mod.UserroleModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'security/user-quota',
        loadChildren: () =>
          import('./security/user-quota/user-quota.module').then(
            mod => mod.UserQuotaModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'security/securitypolicy',
        loadChildren: () =>
          import('./security/security-policy/security-policy.module').then(
            mod => mod.SecuritypolicyModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'security/certificate',
        loadChildren: () =>
          import('./security/certificate/certificate.module').then(
            mod => mod.CertificateModule
          )
      },
      {
        path: 'security/kerberos',
        loadChildren: () =>
          import('./security/kerberos/kerberos.module').then(
            mod => mod.KerberosModule
          )
      },
      {
        path: 'security/dataSecurity',
        loadChildren: () =>
          import('./security/data-security/data-security.module').then(
            mod => mod.DataSecurityModule
          )
      },
      {
        path: 'security/hostTrustworthiness',
        loadChildren: () =>
          import(
            './security/host-trustworthiness/host-trustworthiness.module'
          ).then(mod => mod.HostTrustworthinessModule)
      },
      {
        path: 'security/ldapService',
        loadChildren: () =>
          import('./security/ldap-config/ldap-config.module').then(
            mod => mod.LdapConfigModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'security/samlSsoConfig',
        loadChildren: () =>
          import('./security/saml-sso-config/saml-sso-config.module').then(
            mod => mod.SamlSsoConfigModule
          )
      },
      {
        path: 'security/adfsConfig',
        loadChildren: () =>
          import('./security/adfs-config/adfs-config.module').then(
            mod => mod.AdfsConfigModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'license',
        loadChildren: () =>
          import('./license/license.module').then(mod => mod.LicenseModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'network-config',
        loadChildren: () =>
          import('./network-config/network-config.module').then(
            mod => mod.NetworkConfigModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'log-management',
        loadChildren: () =>
          import('./log-management/debug-log/debug-log.module').then(
            mod => mod.DebugLogModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'export-query',
        loadChildren: () =>
          import('./export-query/export-query.module').then(
            mod => mod.ExportQueryModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'settings/tag-management',
        loadChildren: () =>
          import('./tag-management/tag-management.module').then(
            mod => mod.TagManagementModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'settings/system-backup',
        loadChildren: () =>
          import('./settings/system-backup/system-backup.module').then(
            mod => mod.SystemBackupModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'settings/alarm-notify',
        loadChildren: () =>
          import('./settings/alarm-notify/alarm-notify.module').then(
            mod => mod.AlarmNotifyModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'settings/alarm-notify-settings',
        loadChildren: () =>
          import(
            './settings/cyber-alarm-notify/cyber-alarm-notify.module'
          ).then(mod => mod.CyberAlarmNotifyModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'settings/alarm-settings',
        loadChildren: () =>
          import('./settings/alarm-settings/alarm-settings.module').then(
            mod => mod.AlarmSettingsModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'settings/alarm-dump',
        loadChildren: () =>
          import('./settings/alarm-dump/alarm-dump.module').then(
            mod => mod.AlarmDumpModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'settings/snmp-trap',
        loadChildren: () =>
          import('./settings/snmp-trap/snmp-trap.module').then(
            mod => mod.SnmpTrapModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'settings/sftp-service',
        loadChildren: () =>
          import('./settings/sftp-service/sftp-service.module').then(
            mod => mod.SftpServiceModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'settings/ibmc',
        loadChildren: () =>
          import('./settings/ibmc-config/ibmc-config.module').then(
            mod => mod.IbmcConfigModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'settings/system-time',
        loadChildren: () =>
          import('./settings/system-time/system-time.module').then(
            mod => mod.SystemTimeModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'external-associated-systems',
        loadChildren: () =>
          import(
            './external-associated-systems/external-associated-systems.module'
          ).then(mod => mod.ExternalAssociatedSystemsModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'settings/config-network',
        canDeactivate: [CanDeactivateGuard],
        loadChildren: () =>
          import('./settings/config-network/config-network.module').then(
            mod => mod.ConfigNetworkModule
          )
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SystemRouterModule {}
