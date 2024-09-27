/**
 * 全局公共服务
 */
import { Injectable } from '@angular/core';
import { Subject, BehaviorSubject } from 'rxjs';
import { filter, map, debounceTime } from 'rxjs/operators';

export interface Store {
  action: string; // 行为
  state: any; // 数据
}

@Injectable({
  providedIn: 'root'
})
export class GlobalService {
  store$ = new Subject<Store>(); // 订阅初始化后的流
  behaviorUserStore$ = new BehaviorSubject<Store>({ action: '', state: '' }); // 订阅前默认有初始化初始化流
  behaviorPermissionStore$ = new BehaviorSubject<Store>({
    action: '',
    state: ''
  }); // 订阅前默认有初始化初始化流

  message$ = new Subject<Store>(); // 用于消息传递的流

  constructor() {}

  // 发布
  emitStore(source: Store) {
    this.store$.next(source);
  }

  // 订阅，所有流
  onStore() {
    return this.store$.asObservable();
  }

  // 订阅，具体流
  getState(action) {
    return this.onStore().pipe(
      filter((res: any) => res.action === action),
      map(res => res.state)
    );
  }

  // 发布
  emitBehaviorStore(source: Store) {
    this.behaviorUserStore$.next(source);
  }

  // 订阅
  onBehaviorStore() {
    return this.behaviorUserStore$.asObservable();
  }

  // 订阅user用户信息
  getUserInfo() {
    return this.onBehaviorStore().pipe(
      filter((res: any) => res.action === 'userInfo' && res.state)
    );
  }

  setViewPermission(source: Store) {
    this.behaviorPermissionStore$.next(source);
  }

  // 订阅视图权限
  getViewPermission() {
    return this.behaviorPermissionStore$.asObservable().pipe(
      filter((res: any) => res.action === 'viewPermission' && res.state),
      map(res => res.state),
      debounceTime(1)
    );
  }
}
