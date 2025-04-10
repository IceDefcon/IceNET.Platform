//--------------------------------------------
// Essential Sensors for a Six-Arm Drone
//--------------------------------------------

1. Barometer (Altimeter) - BMP388 / MS5611
✅ Why? Measures altitude using air pressure. Essential for altitude hold, terrain following, and autonomous landing.
🔧 Placement Tip: Mount away from propeller airflow to avoid pressure fluctuations.

2. GPS Module - Ublox NEO-M8N / ZED-F9P
✅ Why? Provides position data for navigation, autonomous flight, and GPS hold mode.
🔧 Placement Tip: Keep it far from other electronics to reduce interference.

3. Optical Flow Sensor - PMW3901 / PX4FLOW
✅ Why? Helps with precise position hold, especially when flying indoors or in GPS-denied environments.
🔧 Placement Tip: Must face downward for ground tracking.

4. LiDAR or Ultrasonic Sensor - TF-Luna / TF-Mini
✅ Why? Used for obstacle avoidance and terrain following.
🔧 Placement Tip: Front-facing (for collision avoidance) or downward (for precision landing).

5. Current & Voltage Sensor - Mauch / INA226
✅ Why? Monitors battery voltage and power consumption, helping prevent mid-air battery failures.
🔧 Placement Tip: Install between the battery and power distribution board.

6. IMU Temperature Sensor - TMP102 / Internal IMU Sensor
✅ Why? IMUs are temperature-sensitive, and fluctuations cause drift. Some IMUs have built-in temperature sensors, but external sensors can provide better monitoring.
Advanced Sensors for High-Performance Drones

7. Dual IMU System (Redundancy) - ICM-20948 / BNO055
✅ Why? Adds fault tolerance—if one IMU fails, the backup ensures stable flight.
🔧 Placement Tip: Install the second IMU on a different part of the frame to detect vibrations better.

8. Blackbox Logging Module - OpenLog / Flight Controller Built-in Logging
✅ Why? Records flight data, useful for analyzing crashes and tuning performance.
🔧 Placement Tip: Attach to flight controller’s UART or SPI port.

9. Airspeed Sensor - MS4525DO (Fixed-Wing Drones Only)
✅ Why? Measures wind speed for autonomous navigation and efficiency optimization.
🔧 Placement Tip: Front-facing, away from prop wash.
