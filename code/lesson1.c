// SPDX-License-Identifier: zlib-acknowledgement

/*
 * Embedded system special-purpose. More restrictions/constraints in software (deterministic, real time), 
 * hardware (speed, code, ram), debugging (limited hardware breakpoint timing errors), 
 * mass deployment/maintanence
 * Schematic first. Larger component, generally more important
 * Learning a new MCU is like a new programming language (a DSP can be another name for an MCU)
 * Don't hardcode memory mapped addresses. Use BSP or create own.
 * Embedded systems is a team sport of EE and software. 
 * Could be bug in software or hardware (power supply failing, failed to get cpu errata).
 * Therefore, most embedded systems are poorly designed as you often only know half
 * Write hardware upbringing tests when waiting for hardware
 * Embedded systems interact with our world, not just our screens
 *
 * Progess through: https://embedded.fm/blog/ese101
 * Circuit simulators: https://www.falstad.com/circuit/circuitjs.html
 * Button debouncing: https://hackaday.com/2015/12/09/embed-with-elliot-debounce-your-noisy-buttons-part-i/
 * 
 */
