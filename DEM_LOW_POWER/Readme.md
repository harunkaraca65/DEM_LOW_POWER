# STM32 Düşük Güç Modları Döngüsü - Algoritma Akışı
# STM32 Low-Power Modes Cycle - Algorithm Flow

---

## 🇹🇷 Türkçe

Bu STM32 uygulaması, bir dizi düşük güç modunu (Sleep, Stop, Standby) göstermek için RTC (Gerçek Zamanlı Saat) kesmelerini kullanır.
Program başladığında, donanımı (GPIO, RTC, Saatler) başlatır.
İlk olarak, bir "Standby" modundan uyanıp uyanmadığını kontrol eder.
Eğer Standby modundan uyandıysa, bunu belirtmek için bir LED'i (PD2) 2 saniye boyunca yakıp söndürür ve bayrakları temizler.
Eğer normal bir başlangıçsa, tüm bayrakları temizler ve ana "Run" LED'ini (PA15) yakar.
Sistem, ana `while(1)` döngüsüne girer ve bir buton (PB11) basımını bekler.
Butona basıldığında, düşük güç modları dizisi başlar.
İlk olarak, "Sleep" moduna geçişi belirtmek için bir LED'i (PA15) 2 saniye yakıp söndürür.
Ardından, RTC'yi 2 saniye sonrasına ayarlar ve "Sleep" moduna girer (CPU durur).
RTC kesmesi (2sn sonra) sistemi uyandırır.
Daha sonra, "Stop" moduna geçişi belirtmek için farklı bir LED'i (PD0) 2 saniye yakıp söndürür.
RTC'yi 5 saniye sonrasına ayarlar ve "Stop" moduna girer (daha derin uyku, saatler durur).
RTC kesmesi (5sn sonra) sistemi uyandırır ve sistem saatini yeniden yapılandırır.
Son olarak, "Standby" moduna geçişi belirtmek için üçüncü bir LED'i (PD1) 2 saniye yakıp söndürür.
RTC'yi 10 saniye sonrasına ayarlar ve "Standby" moduna girer (en derin uyku, RAM korunur).
Sistem 10 saniye sonra RTC tarafından uyandırıldığında, "Standby" modundan çıkış bir sıfırlama (reset) tetikler.
Program bu nedenle `main` fonksiyonunun en başından yeniden başlar.
Yeniden başladığında, bu kez "Standby" bayrağını algılar ve (PD2) LED'ini yakıp söndürerek döngüyü tamamlar.

---

## 🇬🇧 English

This STM32 application uses RTC (Real-Time Clock) interrupts to demonstrate a sequence of low-power modes (Sleep, Stop, Standby).
When the program starts, it initializes the hardware (GPIO, RTC, Clocks).
It first checks if it woke up from "Standby" mode.
If it woke from Standby, it flashes an LED (PD2) for 2 seconds to indicate this and clears the flags.
If it is a normal startup, it clears all flags and turns on the main "Run" LED (PA15).
The system enters the main `while(1)` loop and waits for a button press (PB11).
When the button is pressed, the low-power mode sequence begins.
First, it flashes an LED (PA15) for 2 seconds to signal the transition to "Sleep" mode.
It then sets the RTC for 2 seconds and enters "Sleep" mode (CPU stops).
The RTC interrupt (after 2s) wakes the system.
Next, it flashes a different LED (PD0) for 2 seconds to signal the transition to "Stop" mode.
It sets the RTC for 5 seconds and enters "Stop" mode (deeper sleep, clocks stop).
The RTC interrupt (after 5s) wakes the system, and it reconfigures the system clock.
Finally, it flashes a third LED (PD1) for 2 seconds to signal the transition to "Standby" mode.
It sets the RTC for 10 seconds and enters "Standby" mode (deepest sleep, RAM is retained).
When the system is awakened by the RTC after 10 seconds, exiting "Standby" triggers a system reset.
The program therefore restarts from the very beginning of the `main` function.
Upon restarting, it detects the "Standby" flag this time, flashes the (PD2) LED, and completes the cycle.