import { Directive, ElementRef, Input, OnInit } from '@angular/core';
import { includes } from 'lodash';
import { RoleOperationAuth } from '../consts/permission.const';

@Directive({
  selector: '[auiRolePermission]'
})
export class RolePermissionDirective implements OnInit {
  @Input() rolePermission: string;
  roleOperationAuth = RoleOperationAuth;

  constructor(private elementRef: ElementRef) {}

  ngOnInit(): void {
    this.elementRef.nativeElement.style.display = includes(
      this.roleOperationAuth,
      this.rolePermission
    )
      ? ''
      : 'none';
  }
}
