### RoboScribe - Robot Skryba
Witamy w dokumentacji technicznej projektu **RoboScribe**. \n
Jest to robot do pisania po tablicy magnetycznej.

---

## 📂 Struktura Projektu

Poniższe drzewo przedstawia organizację projektu oraz ich przeznaczenie.

<pre>
📦 RoboScribe
 ┣ 📂 docs          # Wygenerowana dokumentacja
 ┗ 📂 firmware      # Kod źródłowy
    ┣ 📂 drivers    # Warstwa HAL
    ┃  ┣ 📂 MPU6050   → Obsługa Akcelerometru/Żyroskopu
    ┃  ┗ 📂 servo     → Obsługa serwomotorów
    ┗ 📂 main       # Aplikacja główna
</pre>