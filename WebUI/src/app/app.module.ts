import { BrowserModule } from "@angular/platform-browser";
import { BrowserAnimationsModule } from "@angular/platform-browser/animations";
import { NgModule } from "@angular/core";
import { RouterModule, Routes } from "@angular/router";
import { FormsModule } from "@angular/forms";
import { NgxLoadingModule, ngxLoadingAnimationTypes } from "ngx-loading";
import { NgbModule } from "@ng-bootstrap/ng-bootstrap";
import { AppComponent } from "./app.component";
import { NavbarComponent } from "./components/navbar/navbar.component";
import { HomeComponent } from "./components/home/home.component";
import { RacedisplayComponent } from "./components/racedisplay/racedisplay.component";
import { ConfigComponent } from "./components/config/config.component";
import { SystemDataComponent } from "./components/system-data/system-data.component";
import { LightsComponent } from "./components/racedisplay/lights/lights.component";
import { EtsdataService } from "./services/etsdata.service";
import { SecureEtsDataService } from "./services/secure-ets-data.service";
import { LoginComponent } from "./components/login/login.component";
import { HttpClientModule } from "@angular/common/http";
import { DatePipe } from "@angular/common";
import { FriendlyRaceStatePipe } from "./pipes/friendly-race-state.pipe";
import { RaceControlComponent } from "./components/racedisplay/race-control/race-control.component";
import { TimeDisplayComponent } from "./components/racedisplay/time-display/time-display.component";
import { DogDetailsComponent } from "./components/racedisplay/dog-details/dog-details.component";

const appRoutes: Routes = [
  { path: "", component: RacedisplayComponent },
  { path: "racedisplay", component: RacedisplayComponent },
  { path: "config", component: ConfigComponent },
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
    FriendlyRaceStatePipe,
    RaceControlComponent,
    TimeDisplayComponent,
    DogDetailsComponent,
  ],
  imports: [
    BrowserModule,
    FormsModule,
    BrowserAnimationsModule,
    RouterModule.forRoot(appRoutes, { useHash: true }),
    NgxLoadingModule.forRoot({
      animationType: ngxLoadingAnimationTypes.rectangleBounce,
      backdropBorderRadius: "4px",
    }),
    NgbModule,
    HttpClientModule,
  ],
  providers: [EtsdataService, SecureEtsDataService, DatePipe],
  bootstrap: [AppComponent],
})
export class AppModule {}
