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

import { RaceDataService } from './services/race-data.service';

const appRoutes: Routes = [
   { path: '', component: HomeComponent },
   { path: 'racedisplay', component: RacedisplayComponent }
];

@NgModule({
  declarations: [
    AppComponent,
    NavbarComponent,
    HomeComponent,
    RacedisplayComponent
  ],
  imports: [
     BrowserModule,
     BrowserAnimationsModule,
     RouterModule.forRoot(appRoutes, { useHash: true }),
     LoadingModule.forRoot({
      animationType: ANIMATION_TYPES.rectangleBounce,
      backdropBorderRadius: '4px'
      }),
      NgbModule.forRoot()
  ],
  providers: [RaceDataService],
  bootstrap: [AppComponent]
})
export class AppModule { } 
