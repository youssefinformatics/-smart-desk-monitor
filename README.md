# Smart Desk Monitor

Real-time ergonomics monitoring system built on Arduino. Detects 
whether a person is at their desk and continuously monitors 
temperature, ambient light, and screen distance — alerting the 
user when conditions fall outside healthy ranges.

## Hardware

| Component | Pin |
|-----------|-----|
| NTC Thermistor (10kΩ, β=3977) | A1 |
| LDR (Light Dependent Resistor) | A0 |
| HC-SR04 Ultrasonic Sensor | Trig: 9, Echo: 8 |
| IR Proximity Sensor (IR-08H) | 7 |
| Passive Buzzer | 3 |
| Status LED | 6 |

## Features

- Presence detection via IR sensor — system activates/sleeps 
  automatically
- Temperature monitoring with Steinhart-Hart NTC calculation
- Ambient light monitoring with configurable dark/bright thresholds
- Screen distance monitoring — buzzer alert if user sits too close
- Periodic 10-minute workspace suggestions via Serial monitor
- Mood classification based on combined temp + light readings
- Audio feedback: distinct tones for startup, activation, warnings, 
  and idle states

## Thresholds (configurable)

| Parameter | Low | High |
|-----------|-----|------|
| Temperature | 18°C | 30°C |
| Light level | 300 (ADC) | 700 (ADC) |
| Screen distance | — | 20 cm |

## Build

Flash `smart_desk_monitor.ino` to any Arduino Uno/Nano via 
Arduino IDE. Open Serial Monitor at 9600 baud for the live 
dashboard.

## PCB

Schematic and PCB layout designed in KiCad. 
Board not yet manufactured — prototype built on breadboard.
