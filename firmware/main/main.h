/**
 * @file main.h
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @author Maciej Nowicki
 * @brief 
 * @version 0.1
 * @date 2026-01-09
 * @ingroup main
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include <stdarg.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_intr_types.h"
#include "esp_timer.h"
#include "esp_vfs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "sdkconfig.h"

#include "protocol_examples_common.h"
#include "MPU6050.h"
#include "file_server.h"
#include "robot_data.h"
#include "SDcard.h"
#include "servo.h"
#include "stepper.h"

#define CORE0   0
#define CORE1   1

#define LEDC_TIMER_0             LEDC_TIMER_0
#define SERVO_CHANNEL_0          LEDC_CHANNEL_0
#define LEDC_TIMER_1             LEDC_TIMER_1
#define SERVO_CHANNEL_1          LEDC_CHANNEL_1

#define BUTTON_GPIO        21
#define GPIO_INPUT_PIN_SEL 1ULL<<BUTTON_GPIO
#define ESP_INTR_FLAG_DEFAULT 0

#define I2C_SDA                     8
#define I2C_SCL                     18
#define I2C_MASTER_NUM              I2C_NUM_0

const float valTOms2 = 9.81/16384;

/// @brief Command read from .gcode file line
typedef struct {
    uint8_t num;    /// Command number - 0, 1 ...
    int16_t a, b;     /// Axes/angles parameters -- x, y lub v, t
    int16_t tool;      /// Flag - which axes were given
}command;

/**
 * @brief Przerwanie sprzętowe (ISR) inicjujące rozpoczęcie wykonywania zadania przez robota.
 * @note Wykorzystuje atrybut IRAM_ATTR w celu szybkiego wykonywania z pamięci RAM i implementuje sprzętowy mechanizm debouncingu.
 * @return void
 */
void start_execution_isr(void);

/**
 * @brief Inicjalizuje i konfiguruje magistralę I2C w trybie Master.
 * @param bus_handle Wskaźnik na uchwyt magistrali (i2c_master_bus_handle_t), do którego zostaną przypisane ustawienia nowego połączenia.
 * @return void
 */
void i2c_master_init(i2c_master_bus_handle_t *bus_handle);

/**
 * @brief Oblicza liczbę kroków silnika krokowego wymaganą do przemieszczenia robota o zadany dystans w linii prostej.
 * @param dx_mm Przemieszczenie w osi X (w milimetrach).
 * @param dy_mm Przemieszczenie w osi Y (w milimetrach).
 * @return Wymagana liczba kroków (uint32_t) uwzględniająca średnicę koła, mikrokroki oraz parametry kompensacyjne.
 */
uint32_t calculateMoveSteps(float dx_mm, float dy_mm);

/**
 * @brief Wyznacza liczbę kroków niezbędną do wykonania obrotu w kierunku docelowych współrzędnych oraz oblicza nowe kąty orientacji.
 * @param dx_mm Różnica wektora docelowego w osi X (w milimetrach).
 * @param dy_mm Różnica wektora docelowego w osi Y (w milimetrach).
 * @param current_heading Obecna orientacja kątowa robota (w radianach).
 * @param new_heading_out Wskaźnik, pod którym zostanie zapisany nowy kąt orientacji w przestrzeni (w radianach).
 * @param rot_angle_out Wskaźnik, pod którym zostanie zapisany względny kąt, o jaki maszyna musi się fizycznie obrócić.
 * @return Wymagana liczba kroków napędu obrotowego (uint32_t).
 */
uint32_t calculateRotationSteps(float dx_mm, float dy_mm, float current_heading, float *new_heading_out, float *rot_angle_out);

/**
 * @brief Przeprowadza sekwencję pozycjonowania maszyny do zadanego punktu, generując uprzednio odpowiedni obrót, a następnie ruch postępowy.
 * @param x Docelowa współrzędna osi X (w milimetrach).
 * @param y Docelowa współrzędna osi Y (w milimetrach).
 * @param speedHz Prędkość wykonywania obrotów i przemieszczania, wyrażona w częstotliwości uderzeń silnika (w hercach).
 * @return void
 */
void navigateTo(float x, float y, uint32_t speedHz);

/**
 * @brief Główne zadanie systemu FreeRTOS obsługujące ręczne sterowanie robotem na podstawie komend odczytywanych z asynchronicznej kolejki.
 * @details Zadanie nasłuchuje komunikatów i generuje odpowiednie polecenia ruchu roboczo-obrotowego dla platformy jezdnej, osi ramion robota oraz osprzętu narzędziowego.
 * @param pvParameters Standardowy wskaźnik konfiguracyjny (void*) przekazywany przez scheduler FreeRTOS przy tworzeniu zadania.
 * @return void
 */
void manual_control_task(void *pvParameters);

/**
 * @brief Zadanie systemu FreeRTOS powołane do ciągłego nawiązywania i utrzymywania połączenia sieciowego i uruchomienia serwera plików HTTP.
 * @param pvParameters Wskaźnik łańcucha znaków określający główną ścieżkę dla wbudowanego serwera plików (zwykle powiązany z punktem montowania partycji SD).
 * @return void
 */
void create_server_task(void *pvParameters);

/**
 * @brief Monitorujący proces środowiska FreeRTOS badający na żywo wejścia cyfrowe w celu weryfikacji sygnałów awaryjnych (np. przycisk stopu lub utrata bezpieczeństwa obwodu).
 * @details W zależności od zmiany sygnału w locie decyduje on o dynamicznym usypianiu (vTaskSuspend) lub wznawianiu (vTaskResume) kluczowych zadań ruchowych takich jak interpretacja G-Code, czy sterowanie bezpośrednie z IMU.
 * @param pvParameters Opcjonalny wskaźnik danych parametrycznych zadania FreeRTOS.
 * @return void
 */
void gpio_monitor_task(void *pvParameters);