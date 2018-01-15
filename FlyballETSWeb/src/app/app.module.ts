import { BrowserModule } from '@angular/platform-browser';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { DomSanitizer } from '@angular/platform-browser';
import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { HttpModule } from '@angular/http';
import { FormsModule } from '@angular/forms';
import { LoadingModule, ANIMATION_TYPES } from 'ngx-loading';
import { NgbModule } from '@ng-bootstrap/ng-bootstrap';
import { AppComponent } from './app.component';
import { NavbarComponent } from './components/navbar/navbar.component';
import { HomeComponent } from './components/home/home.component';
import { RacedisplayComponent } from './components/racedisplay/racedisplay.component';
import { ConfigComponent } from './components/config/config.component';

import { SystemDataComponent } from './components/system-data/system-data.component';
import { LightsComponent } from './components/lights/lights.component';
import { EtsdataService } from './services/etsdata.service';
import { SecureEtsDataService } from './services/secure-ets-data.service';
import { LoginComponent } from './components/login/login.component';
import { HttpClientModule } from '@angular/common/http';
import { CrossingTime } from './pipes/crossing-time.pipe';
import { DatePipe } from '@angular/common';


const appRoutes: Routes = [
   { path: '', component: HomeComponent },
   { path: 'racedisplay', component: RacedisplayComponent },
   { path: 'config', component: ConfigComponent }
];

@NgModule({
  declarations: [
    AppComponent,
    NavbarComponent,
    HomeComponent,
    RacedisplayComponent,
    ConfigComponent,
    SystemDataComponent,
    LightsComponent,
    LoginComponent,
    CrossingTime
  ],
  imports: [
     BrowserModule,
     FormsModule,
     BrowserAnimationsModule,
     RouterModule.forRoot(appRoutes, { useHash: true }),
     LoadingModule.forRoot({
      animationType: ANIMATION_TYPES.rectangleBounce,
      backdropBorderRadius: '4px'
      }),
      NgbModule.forRoot(),
      HttpClientModule
  ],
  providers: [EtsdataService,SecureEtsDataService, DatePipe],
  bootstrap: [AppComponent]
})
export class AppModule { } 
