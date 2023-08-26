## FlyballETS rev.S by simonttp78

Ten projekt bazuje na systemie treningowym Flyball ETS wykonanym przez vyruz1986.
ETS oznacza Electronic Training System (Elektroniczny System Treningowy) i jest to adaptacja EJSa (Electronic Judging System - Elektroniczny System Sędziowski) do celów treningowych sportu Flyball.

## Przygotowanie ETS do pracy

Zalecane jest rozstawienie części aktywnej i pasywnej na odległość ok. 1.4 - 1.5m. Większa odległość może powodować "szum sensorów" podczas przebiegania psa.
Max. rozstawienie nie powinno przekraczać 1.7m.
- Po wstępnym rozstawieniu stóp nałożyć część aktywną i wstępnie dokręcić zachowując poziom przy użyciu "poziomicy / oczka"
- Skorygować ustawienie stopy tak, aby linia startu/mety pokrywała się z linią czujników od strony handlerów. Docisnąć stopę
- Skorygować ustawienie poziomu części aktywnej oraz jej kąt względem toru, tak aby wiązka była prostopadła do toru
- Zamocować wstępnie część pasywną do stopy
- Włączyć diody laserowe służące do pozycjonowania i skorygować ustawienie części pasywnej, tak aby diody celowały w stosowne punkty na części pasywnej
- Wskazania poziomicy na części pasywnej powinny być podobne do tych na części aktywne, ale nie muszą być IDEALNIE w poziomie. Co więcej w przypadku nierównego terenu, gdy część aktywna jest wyżej niż pasywna, to aktywną należy pochylić lekko w kierunku pasywnej, a pasywną odchylić lekko do tyłu. Analogicznie, jeśli część aktywna jest niżej niż część pasywna, to część aktywną należy pochylić lekko w tył, a część pasywną w przód (w kierunku części aktywnej). Dzięki temu pomimo nierównego terenu górne/dolne wiązki nie będą celowały nad lub pod "lusterka" części pasywnej.

Jeśli w stanie gotowości (READY, po resecie) górne białe światło błędu świeci się lub mruga to znaczy, że czujniki nie trafiają w lusterka części pasywnej i należy skorygować ustawienie.
Podobna sytuacja może mieć miejsce w trakcie deszczu / mżawki / mgły, wówczas należy przetrzeć lusterka oraz same czujniki miękką chłonną szmatką.
Czujniki mogą też zostać zakłócone w przypadku bezpośredniego jasnego oświetlenia (słońce), dlatego należy ustawiać część aktywną tyłem do słońca (szczególnie rano oraz wieczorem, gdy słońce jest nisko na horyzoncie). Oczywiście należy także stosownie ustawić kierunek biegu ("side switch").
Im lepsza dokładność ustawienia czujników, tym bardziej niezawodne jest działanie systemu. Doświadczenie pokazało, że jeśli w trakcie treningu często pojawiają się "dziwne/nienaturalne" wyniki, to niemal zawsze jest to spowodowane problemem z wypozycjonowaniem systemu.
[Film z opisem pozycjonowania](https://youtu.be/arwH5YqZwjc)

## Aktualizacja Firmware'u ETS

#### Sprawdzenie, pobranie nowej wersji firmware ETS

1. Uruchomić ETSa i odczytać wyświetlaną w trakcie uruchamiania wersję firmware (np. "ver. 1.2.1"). Od wersji 1.9.0 informacja o wersji
   firmware widoczna jest także w stopce interfejsu www/WiFi.
2. Otworzyć stronę z aktualizacjami firmware'u, dostępną pod adresem [Releases](https://github.com/simonttp78/FlyballETS-Software/releases)
3. Sprawdzić najnowszą dostępną wersję oprogramowania. Jeśli jest nowsza od zainstalowanej, to zalecania jest aktualizacja
4. Pobrać plik z rozszerzeniem ".bin", który ma w nazwie wersję oprogramowania. Jeśli plik jest niewidoczny kliknąć najpierw w nagłówek "Assets"

#### Wykonanie aktualizacji firmware
Wspierana/testowana wersja przedlądarki do wykonania aktualizacji to Chrome. Safari też działa, za wyjątkiem paska postępu (trzeba cierpliwie czekać).
1. Uruchomić ETSa
2. Połączyć komputer z siecią WiFi nadawaną przez ETSa (domyślna nazwa sieci zaczyna się od "FlyballETS", domyślne hasło "FlybaETS.1234")
3. Otworzyć w Chrome stronę główną systemu ETS przez wpisanie w przeglądarkę [flyballets.local] lub [192.168.20.1]
4. Kliknąć w zakładkę "Firmware update"
5. Zalogować się używając loginu "Admin" oraz hasła (domyślne to "FlyballETS.1234). Jeśli okno logowania się nie pojawi, zalogować się najpierw w zakładce "Configuration" i wrócić do zakładki "Firmware update".
6. Po zalogowaniu domyślnie będzie zaznaczona opcja "Firmware", tak zostawić. Kliknąć "Wybierz plik" i wskazać pobrany plik firmware z rozszerzeniem ".bin"
7. Aktualizacja rozpocznie się automatycznie. Poprawnie wykonana zakończy się napisem "OTA Success". ETS zrestartuje się automatycznie.


## Wyświetlacz LCD
- W trakcie uruchamiania na ekranie LCD wyświetlana jest wersja firmware'u
- Dokładność wyświetlanych czasów może być z dokładnością do 3 lub 2 miejsc po przecinku. Zmiana przy pomocy przycisku "mode / tryb"
- Clean Time "CT" (Czysty Czas) ma sens wyłącznie dla bezbłędnych biegów. W innym prazypadku pojawi się "n/a".
- Jeśli pies zrobił niemierzalny błąd zmiany ("fault"), to czas wcześniejszego psa jest nieważny i poprzedzony tagiem "#"
- W prawym górnym rogu wyświetlany jest nr biegu. Ten numer zapisany jest też na karcie SD. Służy do identyfikacji biegu
- Wartość numeryczna ze znakiem "%" to przybliżony stan naładowania baterii
- Symbol ">" lub "<" w prawym dolnym rogu oznacza kierunek nabiegania psa. Zalecane jest ustawianie aktywnej części ETSa po prawej stronie patrząc w kierunku boksu.
- Jeśli pies miał błąd, który został poprawiony, czyli pies będzie miał więcej niż jeden czas w danym biegu, wówczas na wyświetlaczu czasy te będą się wyświetlały naprzemiennie z oznaczeniem na końcu, który czas jest wyświetlany (*1 - pierwszy dostępny czas, *2 - drugi czas z poprawiania błędu)
- Jeżeli funkcja "Bez powtórek / Re-runs OFF" jest aktywna, to na wyświetlaczu pojawiają się sufixy "*X"
- W prawej dolnej części dostępne są znaczniki informujące o detekcji sygnału GPS ("G"), aktywnym WiFi ("W") oraz włożeniu karty SD ("sd")
- Przełączanie trybu pracy (mode: FCI lub NAFA) powoduje chwilowe wyświetlenie na ekranie informacji, który tryb został wybrany. Tryb pracy jest zapisywany w pamięci.
- Pojawienie się w miejscu wyników pierwszego psa wartości "<- -> ERROR" przy jednoczesnym zapaleniu 4 świateł błędów oznacza, że system wykrył nieprawidłowy odczyt czujników spowodowany najprawdopodobniej złym ustawieniem kierunku biegu (zmina przyciskiem side switch lub w menu Configuration interfejsu www).


## Pilot
Zalecane jest, aby bateria pilota była włożona tylko na czas treningu. To zdecydowanie wydłuża jej żywotność.
Funkcje przycisków pilota:
- 1 START / STOP
- 2 RESET
- 3, 4, 5, 6 w trakcie biegu -> ręczne oznaczenie błędu psa (odpowiednio 1, 2, 3, 4) np. gdy zostanie upuszczona piłka
- 3, 4, 5, 6 w trybie gotowości (READY, po resecie) -> wybór, ile psów będzie brało udział w danym biegu
- długie naciśnięcie (puścić przycisk po ok. 2s) 3 w trybie gotowości (READY) -> włączenie/wyłączenie funkcji "Bez powtórek / Re-runs OFF"
- długie naciśnięcie (puścić przycisk po ok. 2s) 4 w trybie gotowości (READY) -> przełączenie między sekwencją startową NAFA i FCI (funkcja przycisku "Tryb / Mode" na obudowie przed wersją oprogramowania v1.6.0)
UWAGA! Sygnał pilota dociera do systemu z opóźnieniem. To znaczy, że bardzo krótkie naciśnięcie przycisków nie będzie dawało reakcji (syngał nie dotrze to procesora).
Dla prawidłowego działania trzeba przytrzymać przycisk pilota przez ok. 0,5s. Można odnieść mylne wrażenie, że mocne przyciskanie "pomaga". To nie jest prawdą. Nie liczy się siła nacisku, a czas jego trwania.


## Przyciski obudowy
#### Od strony boksu:
- Podświetlany włącznik/wyłącznik

#### Od strony handlerów
- Płaski przycisk "Laser".
  Krótkie przyciśnięcie aktywuje diody służące to pozycjonowania systemu. Domyślny czas działania to 60s. Modyfikowany w menu "Configuration" (WiFi)
  Długie przyciśnięcie włącza/wyłącza funkcję WiFi
- Wypukły przycisk "Tryb / Mode"
  Krótkie przyciśnięcie zmienia dokładność wyświeltanych wyników między setnymi lub tysięcznymi częściami sekundy.
  Długie przyciśnięcie przełącza kierunek biegu ("side switch"): normalny (aktywna jednostka po prawej), lub odwrócony/inverted jeśli aktywna jednostka jest po lewej


## Bateria / ładowanie
Aby zapewnić żywotność baterii:
- Nie zaleca się przechowywania aktywnej jednostki w temperaturach poniżej 10st.C, natomiast trening jak najbardziej można prowadzić w temperaturach do 0st.C.
- Jeśli w trakcie treningu nie jest wykorzystywana funkcja WiFi, to można ją wyłączyć (długie przyciśniecie płaskiego przycisku "laser"). Domyślnie WiFi jest zawsze włączone.
- Pomimo zabezpieczeń nie jest zalecane, aby często była używana z poniżej 15% poziomu naładowania
- Baterie LiIon nie należy często ładować bardzo długo (do maksymalnej pojemności). Lepiej, aby ładowanie zakończyć w momencie, gdy niebieska dioda ładowania (nad gniazdem ładowania) zgaśnie. Nie jest niczym złym, gdy odłączy się zasilacz np. 2h od momentu zgaśnięcia diody, ale częste długie ładownie (np. przez noc) jest nizalecane
- W trakcie ładowania baterii nie można korzystać z ETSa. Dopiero po odłączeniu zasilacza możliwe jest uruchomienie systemu


## GPS
- Oczywiście działa wyłącznie na zewnątrz. W przypadku zimowych treningów w zamkniętych / ograniczonych pomieszczeniach, niestety nie przydaje się
- Aktualnie służy głównie do synchronizacji czasu, który wykorzystywany jest przy zapisywaniu danych na karcie SD
- Jeśli komunikacja z satelitami GPS nie jest możliwa, to system zakłada pseudo datę/czas jako 2021-01-01 13:00:00
- W przyszłości być może posłuży do synchronizacji czasu w między dwoma ETSami w trybie "sparing"


## Karta SD
- Zapisywane są na niej dane w treningów, przy czym jeśli ETS nie jest wyłączany nie nastąpiły żadne resety, to wszystkie biegi z treningu powinno być w jednym wspólnym pliku CSV.
- Pliki mają tworzone unikatowe tagi. Nowy tag tworzony jest z pierwszym zakończonym biegiem po resecie / włączeniu systemu. Tak to kolejna liczba z zakresu 1 do 9999.
- Można wyzerować tag przez usunięcie pliku tag.txt z folderu głównego. Należy wówczas usunąć także wszystkie pliki z danymi z wcześniejszych treningów
- Karta SD powinno być sformatowana w FAT32. Pliki z danymi z wyścigu zapisywane są w głównym katalogu.
- W katalogu SENSORS_DATA zapisywane są odczyty z czujników. Służą one do analizy ewentualnych błędów / dziwnych zachować systemu i są bezcenne do odtwarzania problemów i robienia korekcji.
- Domyślnym separatorem dziesiętnym jest kropka '.' tak jak to widać na wyświetlaczu LCD czy przez interfejs WiFi. Może to być jednak kłopotliwe w przypadku pliku CSV na karcie SD, bo odczytując plik w arkuszu kalkulacyjnym w krajach, gdzie separatorem dziesiętnym jest przecinek ',' wartości liczbowe nie zostaną automatycznie przekonwertowane. Z tego powodu w menu Configuration interfejsu www (WiFi) jest możliwość aktywowania funkcji "Używaj przecinka ',' jako separatora dziesiętnego w pliku CSV"
- Włożenie lub wyjęcie karty SD powoduje restart ETSa, oczywiście o ile był włączony


## Światła
5 świateł jest wykorzystywanych zarówno do sekwencji startowej jak i oznaczania błędów psów. Możliwe jest ustawienie sekwencji startowej stosowanej przez NAFA (USA) używając przycisku "Tryb / Mode".
Krótkie mrugnięcie górnego białego światła może oznaczać:
- ręczne zatrzymanie biegu przyciskiem STOP
- reset biegu przyciskiem RESET
- w stanie GOTOWOŚCI / READY problem w wypozycjonowaniu ETSa lub wilgość / krople na czujnikach lub lusterkach.
Przy źle wypozycjonowanym ETSie w stanie GOTOWOŚĆ / READY górne biało światło może cały czas być zapalone, co sygnalizuje potrzebę korekty ustawienia.
- W przypadku detekcji nieprawidło ustawionego kierunku biegu zaświecą się 4 światła błędów jednocześnie, a na wyświetlaczu pojawi się "<- -> ERROR" w miejscu wyników pierwszego psa (zmina przyciskiem side switch lub w menu Configuration interfejsu www).


## Interfejs www (WiFi)
Maksymalnie można podłączyć 8 klientów Wifi jednocześnie, jednakże ze względów wydajnościowych lepiej nie przekraczać ilości 4 klientów jednocześnie szczególnie, że w praktyce jedna osoba steruje ETSem i prezentuje wyniki handlerom.

<b>UWAGA użytkownicy iOSa!!!</b>
Bez poniższej zmiany, próba połączenia z ETSem będzie powodowała ciągłe restarty systemu.

Od wersji iOS 15 wymagane jest wyłączenie w Ustawieniach jednej funkcji:
Ustawienia => Safari => Zaawansowane => Experimental Features => NSURLSession WebSocket
Link do filmu, gdzie jest to pokazane: [Jak skonfigurować telefon z iOS](https://youtu.be/ohZqT0l43w4)

Dodatkowo po nawiązaniu połączenia z siecią WiFi nadawaną przez ETS należy wejść w opcję:
Ustawienia => Wi-Fi => przy sieci FlyballETS nacisnąć "i" w kółku, a następnie wyłączyć:
- Prywatny adres Wi-Fi
- Ograniczaj śledzenie adresu IP

#### Strona główna (Race)
Na tej stronie dostępne te same funkcje sterowania co przy pomocy pilota z tą różnicą, że jest jeden przycisk zmieniający funkcję (start, stop, reset). Podobnie jest z informacjami z wyświetlacza LCD za wyjątkiem informacji o włożeniu karty SD.
Ręczne włączanie błędu psa można wykonać przez naciśnięcie przycisku "Błąd / Fault" w sekcji danego psa. Podświetli się on wówczas na czerwono.

#### Strona konfiguracji (Configuration)
Po zalogowaniu przy użyciu loginu "Admin" i hasła (domyślnie "FlyballETS.1234) i wciśnięciu przycisku Login pojawia się menu z rzadziej zmienianymi parametrami systemu, takimi jak nazwa sieci WiFi, hasło do sieci, hasło administratora, czas załączenia diod laserowych (wyłączą się automatycznie, jeśli zostanie wystartowany bieg), kierunek biegu, typ sekwencji startowej (FCI lub NAFA), dokładność wników (2 lub 3 cyfry po przecinku) oraz separator dziesiętny w pliku CSV (kropka lub przecinek).

#### Strona Firmware update
Została opisane na początku instrukcji w punkcie dotyczącym aktualizacji oprogramowania systemowego.


## Zgłaszanie usterek oprogramowania
System wyposażony jest w dwie diody w okolicach wyświetlacza LCD. Pozwala to tak sfilmować zmiany psów, aby jednocześnie widzieć reakcję systemu (przecięcia wiązek). W przypadku wykrycia dziwnego zachowania, w szczególności niespodziewanych wartości na wyświetlaczu, bardzo proszę o przesłanie mi:
- zrzut ekranu z telefonu tak, aby był widoczny status systemu na dole ekranu (tam gdzie jest poziom baterii, Race ID, etc.)
- pliku z karty SD z wynikami (początek nazwy pliku to numer, taki sam jak pierwszy człon Race ID wyświetlany na dole ekranu telefonu,
  np. Race ID: 14 - 1 --> nazwa pliku  zaczyna się od 0014)
- plik z karty SD z odczytami sensorów z katalogu SENSORS_DATA, którego początek nazwy (numer) pokrywa się z numerem pliku z wynikami (np. 0014).
Wysyłanie zgłoszeń o błędach preferuję w formie [Issue](https://github.com/simonttp78/FlyballETS-Software/issues) bezpośrednio w github.
Alternatywnie proszę o kontakt bezpośredni.
Dla wzrokowców zamieszam link do fimlu ;-): [Jak zebrać logi z ETS](https://youtu.be/VitzgYN1oZY)


## Zgłaszanie pomysłów na nowe funkcjonalności
Jak najbardziej mile widziane. Podobnie jak z błędami, bezpośrednio do mnie lub jako [Issue](https://github.com/simonttp78/FlyballETS-Software/issues) w github.


## Informacje ogólne
Ponieważ konstrukcja systemu bazuje na zasadzie działania EJSa, to dziedziczy także jego ograniczenia, więc trzeba pamiętać, że:
- Większość błędów zmiany jest niemierzalna i w zależności od prędkości i wielkości psa może sięgać nawet -0.10s
- Analogicznie ma się sprawa z bezbłędnymi krosami. System analizuje wyłącznie przecięcia wiązek, ale nie wie, który pies przecina linię startu/mety lub kiedy dokładnie to robi, bo drugi pies już mógł "zakryć" wiązki. Z tego też powodu większość krosów do ok. 0.10s będzie rozpoznawany jako "OK" lub "ok", przy czym małe "ok" jest w większości przypadków nieco gorszym krosem.
- Wyjątek od powyższej reguły stanowi bardzo mały kros, poniżej 6ms. Wówczas z ograniczeń fizycznych wiadomo, że po przecięciu linii kierunku (tej od boksu) to nabiegający pies musiał przeciąć linię startu mety (powracający nie zdążyłby tego zrobić). W takiej sytuacji system wyświetli "Perfect".
- Podobnie jak w EJSie błąd psa powoduje, że czas psa poprzedniego jest zakłamany i nie należy go uwzględniać w statystykach
- Clean Time ma sens wyłącznie dla bezbłędnych przebiegów, dlatego w innym przypadku jest brak wartości "n/a"

Pomimo tych wad, które można częściowo obejść, ale to już wykracza poza zakres tej instrukcji, ciągle mamy wiele pozytywnych funkcji czasami niedostępnych w systemach EJS, np.:
- kompaktowość, łatwość instalacji
- sterowanie przez WiFi
- łatwa aktualizacja firmware'u
- automatyczna detekcja błędów zmiany
- możliwość konfigurowania ilości trenujących psów oraz czy trening jest z powtórkami czy bez
- wygodne zapisywanie danych na karcie SD w formacie CSV
