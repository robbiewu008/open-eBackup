import { Injectable } from '@angular/core';
import {
  ActivatedRouteSnapshot,
  CanActivate,
  CanActivateChild,
  CanLoad,
  Route,
  Router,
  RouterStateSnapshot,
  UrlSegment,
  UrlTree
} from '@angular/router';
import { isEmpty } from 'lodash';
import { Observable } from 'rxjs';
import { CookieService } from '../services/cookie.service';

@Injectable({
  providedIn: 'root'
})
export class RedirectGuard implements CanActivate, CanActivateChild, CanLoad {
  constructor(private cookieService: CookieService, private router: Router) {}

  canActivateChild(
    childRoute: ActivatedRouteSnapshot,
    state: RouterStateSnapshot
  ):
    | boolean
    | UrlTree
    | Observable<boolean | UrlTree>
    | Promise<boolean | UrlTree> {
    return this.canAccess();
  }

  canActivate(
    route: ActivatedRouteSnapshot,
    state: RouterStateSnapshot
  ):
    | boolean
    | UrlTree
    | Observable<boolean | UrlTree>
    | Promise<boolean | UrlTree> {
    return this.canAccess();
  }

  canLoad(
    route: Route,
    segments: UrlSegment[]
  ): boolean | Observable<boolean> | Promise<boolean> {
    return this.canAccess();
  }

  canAccess() {
    const userId = this.cookieService.get('userId');
    if (!isEmpty(userId)) {
      return true;
    }
    this.router.navigateByUrl('/login');
    return false;
  }
}
