import {
  Directive,
  ElementRef,
  EventEmitter,
  Input,
  OnDestroy,
  OnInit,
  Output
} from '@angular/core';
import { Subject } from 'rxjs';
import { map, takeUntil } from 'rxjs/operators';
import { OperateItems } from '../consts/permission.const';
import { GlobalService } from '../services';

@Directive({
  selector: '[pmpermission],[pm-permission]'
})
export class PermissionDirective implements OnInit, OnDestroy {
  @Input() pmOperation: string;
  @Output() pmPermissionChange = new EventEmitter();

  permissionMap: any = {};
  destroy$ = new Subject<void>();

  constructor(
    private globalService: GlobalService,
    private elementRef: ElementRef
  ) {}

  ngOnInit(): void {
    this.globalService
      .getViewPermission()
      .pipe(
        map(res => {
          const permissionMap = res || ({} as any);
          return !!permissionMap[OperateItems[this.pmOperation]];
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        this.elementRef.nativeElement.style.display = res ? '' : 'none';
        this.pmPermissionChange.emit(res);
      });
  }

  ngOnDestroy(): void {
    this.destroy$.next();
    this.destroy$.complete();
  }
}
