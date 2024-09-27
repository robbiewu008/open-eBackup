import { CommonModule } from '@angular/common';
import {
  Directive,
  EventEmitter,
  Input,
  NgModule,
  OnDestroy,
  OnInit,
  Output
} from '@angular/core';
import { Event, NavigationEnd, Router } from '@angular/router';
import { MenuComponent } from '@iux/live';
import { DataMap, RouterUrl } from '../consts';
import { AppUtilsService } from '../services/app-utils.service';

@Directive({
  selector: 'auto-active-menu'
})
export class AutoActiveMenuDirective implements OnInit, OnDestroy {
  routerEventsSubscription;
  routerLinkMenus = [];

  @Input('menu') menuInstance: MenuComponent;
  @Output() activeMenuChange = new EventEmitter();

  constructor(public appUtilsService: AppUtilsService, private router: Router) {
    this.routerChange();
  }

  ngOnInit() {
    this.menusChange();
    this.initRouterLinkMenus(this.menuInstance.lvMenus, this.routerLinkMenus);
    this.activeMenuItem(this.router.url);
  }

  routerChange() {
    this.routerEventsSubscription = this.router.events.subscribe(
      (event: Event) => {
        if (event instanceof NavigationEnd) {
          this.activeMenuItem(event.url);
        }
      }
    );
  }

  menusChange() {
    this.menuInstance.lvMenusChange.subscribe(() => {
      this.routerLinkMenus = [];
      this.initRouterLinkMenus(this.menuInstance.lvMenus, this.routerLinkMenus);
      this.activeMenuItem(this.router.url);
    });
    this.menuInstance.lvItemClick.subscribe(res => {
      // 用于监听修改网络是否进入了修改状态，如果进入则不能跳转到其他路由
      if (
        this.appUtilsService.getCacheValue('networkModify', false) !==
          DataMap.networkModifyingStatus.normal.value &&
        this.router.url === '/system/settings/config-network' &&
        !res.data.expanded &&
        res.data.id !== 'settings'
      ) {
        this.activeMenuItem(this.router.url);
      }

      // 用于点击保护和数据利用左边菜单时根据默认路由触发卡片排序跳转逻辑
      if (
        res.data?.childrenLink &&
        res.data.childrenLink.includes(this.router.url)
      ) {
        if (
          [
            RouterUrl.ProtectionHostAppOracle,
            RouterUrl.ExploreCopyDataOracle,
            RouterUrl.ProtectionHostAppMongoDB,
            RouterUrl.ExploreCopyDataMongoDB,
            RouterUrl.ProtectionVirtualizationVmware,
            RouterUrl.ExploreCopyDataVMware,
            RouterUrl.ProtectionVirtualizationKubernetes,
            RouterUrl.ExploreCopyDataKubernetes,
            RouterUrl.ProtectionCloudHuaweiStack,
            RouterUrl.ExploreCopyDataHuaweiStack,
            RouterUrl.ProtectionActiveDirectory,
            RouterUrl.ExploreCopyDataActiveDirectory,
            RouterUrl.ProtectionHostAppFilesetTemplate,
            RouterUrl.ExploreCopyDataFileset,
            RouterUrl.ProtectionStorageDeviceInfo,
            RouterUrl.ExploreCopyDataFileSystem
          ]
            .map(item => String(item))
            .includes(this.router.url)
        ) {
          this.router.navigateByUrl(
            res.data.childrenLink.find(item => item !== this.router.url)
          );
        }
      }
    });
  }

  activeMenuItem(url) {
    let activeItem = this.routerLinkMenus.find(d => {
      if (d.childrenLink) {
        return (
          d.childrenLink.includes(url) ||
          !!d.childrenLink.find(c => url?.includes(c))
        );
      } else {
        return d.routerLink === url && !d.items;
      }
    });
    if (activeItem) {
      this.menuInstance.lvActiveItemId = activeItem.id;
      this.activeMenuChange.emit(activeItem.id);
    } else {
      this.menuInstance && (this.menuInstance.lvActiveItemId = '');
    }
  }

  initRouterLinkMenus(menus, routerLinkMenus) {
    if (!menus) {
      return;
    }

    menus.forEach(d => {
      if (d.routerLink) {
        routerLinkMenus.push(d);
      }

      this.initRouterLinkMenus(d.items, routerLinkMenus);
    });
  }

  ngOnDestroy(): void {
    this.routerEventsSubscription.unsubscribe();
  }
}

@NgModule({
  declarations: [AutoActiveMenuDirective],
  imports: [CommonModule],
  exports: [AutoActiveMenuDirective]
})
export class AutoActiveMenuModule {}
