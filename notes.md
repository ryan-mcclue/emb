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
