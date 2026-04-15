/**
 * @file main.c
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @author Bartosz Chabros
 * @brief 
 * @version 0.1
 * @date 2026-01-09
 * 
 * @copyright Copyright (c) 2026
 */

#include "main.h"

void app_main(void)
{
    // Initialize the system
    System_Init();

    // Main loop
    while (1)
    {
        // Update the system state
        System_Update();
    }

    
}

void System_Init(void)
{
    // Initialize peripherals, sensors, and actuators
    // For example:
    // Motor_Init();
}

void System_Update(void)
{
    // Update the system state
    // For example:
    // Motor_Update();
}

double calculate_distance(double x1, double y1, double x2, double y2)
{
    double a = x2 - x1;
    double b = y2 - y1;

    return sqrt(a*a + b*b);
}

double calculate_angle(double x1, double y1, double x2, double y2)
{
    double a = x2 - x1;
    double b = y2 - y1;

    double angle = atan2(b, a); // atan2 zwraca kąt w radianach z zakresu (-pi, pi) 
    // ujemna wartość oznacza kąt w kierunku przeciwnym do ruchu wskazówek zegara, 
    //a dodatnia wartość oznacza kąt w kierunku zgodnym z ruchem wskazówek zegara
    angle = angle * (180.0 / M_PI); // zamiana na stopnie 
    return angle;
}

