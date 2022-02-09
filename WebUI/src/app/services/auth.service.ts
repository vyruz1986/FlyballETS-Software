import { Injectable } from '@angular/core';
import { HttpClient, HttpParams, HttpHeaders } from '@angular/common/http';
import { User } from '../class/user';

@Injectable()
export class AuthService {

   constructor(private http:HttpClient) {}

   login(user:User){
      let options:HTMLOptionsCollection;
      let authHeader = "Basic " + btoa(user.username + ":" + user.password);
      const headers = new HttpHeaders().set('Authorization', authHeader);
      //return this.http.get("http://192.168.20.1/auth",{ headers });
      return this.http.get("http://"+ window.location.host +"/auth",{ headers });
   }
}
