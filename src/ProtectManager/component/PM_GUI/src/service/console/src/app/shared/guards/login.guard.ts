import { Injectable } from '@angular/core';
import {
  ActivatedRouteSnapshot,
  CanActivate,
  Router,
  RouterStateSnapshot,
  UrlTree
} from '@angular/router';
import { isEmpty } from 'lodash';
import { Observable } from 'rxjs';
import { GlobalService } from '../services/store.service';
import { CookieService } from './../services/cookie.service';

@Injectable({
  providedIn: 'root'
})
export class LoginGuard implements CanActivate {
  constructor(
    private router: Router,
    private globalService: GlobalService,
    private cookieService: CookieService
  ) {}

  canActivate(
    next: ActivatedRouteSnapshot,
    state: RouterStateSnapshot
  ):
    | Observable<boolean | UrlTree>
    | Promise<boolean | UrlTree>
    | boolean
    | UrlTree {
    return this.canAccess();
  }

  canAccess() {
    const userId = this.cookieService.get('userId');
    if (isEmpty(userId)) {
      return true;
    }
    this.router.navigate(['/home']);
    return false;
  }
}
