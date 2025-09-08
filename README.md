# Embedded-systems-Project-Group2

This repository contains the research, design, and development files for our Embedded Systems course project. The project goal is to design and implement a simple Battery Management System (BMS) for a 4-cell lithium-based battery pack.

As a project, we will develop a battery management system for a LiFePO4 battery pack, made up of 4 cells (essentially a 12V system). The BMS is primarily a fail-safe device protecting the battery cells.

BMS Project Initial requirements
MOSFET driven switch (both charging and discharging)
Temperature monitoring (two sensors)
Individual battery cell voltages
Current measurement, including over current protection (10A)
STM32 controller with over/under voltage per cell (2.5V min, 3.65V max. per cell), over/under temperature (charge: 0C to 40C and discharge: -20C to 40C)

For Schematic Capture/PCB Layout, use KiCad
For simulation (you should simulate everything), use LTSpice
