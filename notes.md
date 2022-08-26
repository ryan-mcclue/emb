Embedded Youtubers: 
   * Challenger Tech. (wireless)
   * Bitluni (led matrices)
   * Eddie Amaya (bootloader)
   * something about power and batteries

OTA general term 

without bootloader, monolithic firmware
bootloader and software separate binaries 
 
Could also unpack ELF file, however usually just
a binary file.
    - flashing over over-the-air, e.g. phone connected to ear buds
    - security, e.g. verify firmware integrity
MSP and PSP refer to single stack pointer r13, however in different contexts? This way for scheduling?
https://bitbucket.org/csowter/redkernel/src/master/
https://electronics.stackexchange.com/questions/403967/main-stack-pointermsp-vs-process-stack-pointerpsp

On reset, MSP is value of 0x000. PC is 0x04 which points to reset handler.
The reset handler will init hardware and copy/init variables from rom to ram.

If a bootloader is present, the bootloader will have its own reset handler that when finished doing appropriate tasks,
will then call the applications reset handler and follow the steps outlined above.
So, bootloader and application will each have their own vector table.

Although arm says it starts at 0x00, the memory map of an mcu may be different.
So, typically have boot modes, (i.e. setting boot pin value) to control where the boot address is. 
e.g. setting to 0 will start from flash at 0x020000. This address will be a memory alias for 0x00.
Different memory starting regions will have different permissions that allow for read/write/execute  

Dual bank flash allows to erase in one bank while running code in another. Useful for updates.

 IMPORTANT(Ryan): Inside of system_.c, look for something called vector table offset, e.g. USER_VECT_TAB_ADDRESS, VECT_TAB_OFFSET
 and modify it appropriately

 TODO(Ryan): Investigate how to power board externally through Vin 

 IMPORTANT(Ryan): First thing to get an overview is to look at board pinout. Basically everything is user-configurable IO

 dbus formalise IPC with discrete packet sizes as oppose to byte streams with say pipes 

 DBUS_BUS_SESSION will make bluez unknown

 IMPORTANT(Ryan): By default, we cannot register a name on the system bus
 So, must create a policy file in /etc/dbus-1/system.d/my.bluetooth.client.conf 

  
      <!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
      "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
      <busconfig>

      <!-- alternatively could do user="ryan" -->
      <policy context="default">
        <allow own="my.bluetooth.client"/>
      </policy>

      </busconfig>

dbus built on-top of low-level IPCs like shared memory and sockets
system bus for kernel, i.e for all users
session bus for user
if well known name of the service (org.freedesktop.ModemManager) is not started, dbus will start it for us?
service exposes objects (/org/freedesktop/ModemManager)
objects can have various interfaces (org.freedesktop.DBus.ObjectManager) (form of reverse domain name)
the interface will have methods to access data (GetManagedObjects()), also have properties or signals

our process interacts with dbus-daemon (libdbus) which interacts with systemd (sd-bus)

Often there is a VTOR register which we can use to add an interrupt handler
outside of the startup file, isr_vector = (* long)SCB->VTOR; isr_vector[15] = handler;

If writing directly to registers, need to take into account that some operations may
require a few cycles, e.g. initialising clock. HAL takes care of this.
Further example of needing to know hardware, i.e. using codec register setup
We could inspect instruction cycle count in cortex-m4 manual

Interrupts are generally more efficient. Response time-critical. Allow for power saving.
Not hitting bus as hard which might interfere with other bus masters, e.g. DMA

Polling when duration of operation specific, e.g. sending out 50us pulse
If high throughput, e.g 30000 times a second, context switch will be too much (i-cache, d-cache?)

PWM can be useful to preserve power as something that is on for say 90% of time looks like 100%?

Example embedded serial terminal console commands: factory_reset(), get_battery(), get_dev_id()

Unit and integration tests are to be done in software as we have more control and is faster

UNIT TEST TYPES:
state based testing good for independent modules i.e. operate on the edge of the system, 
e.g. math functions, drivers (gpio; they typically just affect state of registers; this is where setup and teardown functions useful),
storage modules, or literal state machines

1. Perform green-field testing (most likely only white box tests)
Naming based on behaviour, e.g. test_Input0ShouldReturn10, etc. 
(or test_Input0_should_Return10, test_SetPinAsOutput_SHOULD_NotUpdateDirection_WHEN_PinIsNotValid)
Once specifics are done do general, e.g. test_InputNShouldReturnValidResponse
TODO(Ryan): .vimrc Auto-add test verbs as comments in test files


values to test for unsigned as these can be 
problematic working with signed and unsigned: 0x80 (10000000), 0x7f (01111111), 0xff

2. Perform error conditions. How these errors are handled is determined by product requirements,
company policy (all errors are logged to a file) and finally falling back on your own judgement
e.g. test_NegativeInputReturns0, test_InputsThatOverflowReturn0 (test one before limit as well)
(perhaps replace _should_ with _b_ or _w_ to signify test type)
limits.h should be used, and possibly spreadsheet calculations.
(select limits and in-between random values)
If making an assumption about the platform, e.g. sizeof(int), put that in an assertion

3. Finally test the API with black box tests. (depending on the size of the module, the distinction between white/black box tests might not be necessary)
Tests should iterate through multiple invocations

CI means to regularly integrate tested code into mainline repository, i.e. merging into master
Require automated builds and tests to achieve this. 
Although an investment is required, forces a level of discipline required to ensure a codebase
can grow and work well at scale.

TODO(Ryan): When are external devices like JTAG/SWD adapters required?
When the board does not have an integrated/on-board debugger, e.g. Atmel EDBG
On page 19, Section Debug Interface, says exposes SWD interface. 
So, if not on-board/integrated debugger, cannot debug? What exactly does it mean when it has
an onboard debugger, does that mean it supports the JTAG/SWD protocol?

Then says to use JLink JTAG/SWD debugger, but then require a adapter cable for JTAG to SWD?
Mentions about openOCD not being able to reset before debugging, so use JLink. 
Is the JLink just a commercial openOCD? It must be different as its a physical device...

(cortex-m coresight architecture)
Arm debug interface architecture. Implementation is Debug Access Port.
Through the debug port are access ports. Access ports expose various registers. 
Debug port can use various protocols. 
JTAG is available in most silicon (standardised in 1990). SWD requires less exposed pins
than JTAG. SWD/JTAG is both (most commonly seen in modern MCUs)
Require an interfacing MCU, i.e. something to map SWD/JTAG to a USB protocol. This could be
a dongle or an IC on the board. Can use CMSIS-DAP to run on another MCU, however perhaps
easiest to use SEGGER J-Link or ST-Link
Ideally want the debug probe to implement RTOS Awareness, e.g. retrieving threads  

TODO(Ryan): DSP importance.

Writing directly to registers instead of using arduino library is much faster.

1. **STARTUP**:
  1. Compile with BSP linker script and startup file
  2. Obtain ISA, CPU, MCU technical reference manuals.
     Obtain board schematics

MCU has boot up sequence (typically found in memory map section). It seems
that most MCUs have preloaded bootloader. 
Seems bootloaders do two things: copy from ROM to RAM or load from
peripheral (UART, USB, etc). Could also unpack ELF file, however usually just
a binary file.
Initial IP will Reset_Handler from .isr_vector
(One reason for using C is that it maps very closely to executable structure)

In reality, all we need is some C code and a linker script to write a
monolithic firmware for our microcontroller. However, it's recommended to
use a bootloader to decouple.

The CPU architecture will have an exception (a cpu interrupt) model.
Here, reset behaviour will be defined.
For ARMV7 we see that VTOR is 0x000, disable all interrupts/exceptions,
load SP from 0x000 and load PC from 0x0004 (we find that the symbol here:
Reset_Handler, is 1 bit lower than expected as lowest bit of interworking 
address indicates Thumb instructions to be used)
TODO(Ryan): Investigate usage of various binutils programs $(nm), etc. 
For raw bytes: $(xxd main.bin), for structure: $(objdump -t main.axf)
Where is information relating to what the reset handler should do?


Why do our programs contain startup assembly? What are they doing?
CPU has reset vector (what section in docs?)

The MCU may have boot loader in flash memory, or as part of 'internal rom',
i.e. memory not included in the 'user memory'. The boot loader is a bit of a
misnomer as it can also be application-initiated to perform firmware updates.
In addition to a boot loader, the MCU may have other software present, e.g.
driver library, AES, CRC etc. 


boot fuse redirect reset vector?
when flashing data over USB (virtual COM port?), are we instructing the boot
loader to erase/load the data?
so, the bootloader determines what type of flashing program to use, e.g LM
Flasher as it will issue a particular UART, SPI interface command? How does
this 'polling' behaviour align with boot loader name?
so, if we corrupt the boot loader we will need to use some external device to
reflash the boot loader correctly (ICSP pins, in circuit serial programming)
3rd party vendors will create devices that handle the intricacies of this for
us. why do they give us a default bootloader and not let us just flash
directly?
tie resistor to reset pin to allow for toggling?



INVESTIGATE SERIES OF 3 BOOKS: Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers

ORGANISE INTO HEADERS

tiva c series board (led switches, convenient pin-outs, power switches, ports,
etc; arduino is board employing atmega MCU) employs tiva tm4c123g MCU with
DMA, SAR ADC, CAN modules, SPI/UART/I2C (are these features of MCU or board?)
the 32 bit arm cortex-m4 has FPU (a series in phones, m for microcontroller, r
also in some phones)
it is labelled as an evaluation board? (does this refer to the IDCI?)
MCU has eeprom / flash (nand or nor flash; subset of eeprom; so when say ROM,
essentially mean flash) / sram memory.
as often harvard archicture (why?) different memory types employed.
memoy degradation/life cycle dffers between them?
CAN (controller area network), controls nodes (electronic control units). Used
as different ECUs can be added/removed to communicate similarly over the CAN
(used in cars)
board uses stellaris ICDI (ST-link is for stm32 mcus)

precision is number of values (determined by number of bits available)

IMPORTANT EMBEDDED FEATURES: testability, size, power, cost, real time 

The cortex M has I/O ports, e.g. Port A, port B, etc. these ports have a number
of pins on each (so a port is just a collection of pins) 
If parallel, can set all these pins/bits at the same time.
Serial single bit at a time (used for communicating with other devices).
Analog used to measure real world, or output signals
Time another type of I/O.

ISA contains instructions, registers (general, special), addressing modes (how to fetch
operands), memory accesses (x86, arm, powerpc (automobiles), SPARC (servers)) 
Stack allows to write modular code
ARM has link register that holds return address of routine
Status register has particular bits, e.g. Z bit for zero 
Cortex-M executes Thumb instructions
Listing file is combined source and object code  
INSTRUCTION TYPES:
Memory access (mov), ALU (sub, add), control (bne), special (interrupts, etc.)

Without Keil, require LM Flash programmer, some JTAG interface/stelllaris
driver for gdb?
