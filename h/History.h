
/* ************************************************************************************************
Solar Microinverter 240V MOSFET Block 05-19-15 V1.01.08
- Made a function called FlashMemoryCheckSum().  It is strategically placed in main().  It does 
  a checksum of the entire flash section, except the last 16 bytes: 
	MAX_INSTRUCTIONS_IN_FLASH		(0x2BF0/2)	// 0x2c00 = Max flash, however using 0x2BF0 since using 
  last bytes for dwCheckSum var.
  if the calculated checksum is not equalled to what the true stored checksum is, the inverter
  will not start and stays in the infinete loop.  THis Info is being sent in RS-485 
- Added Temperature Throttle Back: Inside SelfRescueAndFaults()  It gets called every 5 Seconds. It 
  works in conjunction with wBoardTemperature which is calculated every 0.2 Sec.  Trigger temperature
  is 85C.  When board temperature reaches Trigger Temperature, it puts inverter in factory mode and
  starts decrementing mpptFactor to 80%, 60%, 40%, 20% and 10% until wBoardTemperature goes below
  Trigger Temperature.  When this happens, inverter gets out of factory mode and starts to ramp up power.
- Added IMI Test Code required by UL-1998.  This happens every 5 Secs inside SelfRescueAndFaults().  Function is called 
	IMI_Test(): It enables IMI_OUTPUT which in turn on Q20 connecting PV- to Earth GND using corresponding
	Z.  This test this PV IMI circuit at reset or power up daily. It waits 80 mSec for stability.  Then
  reads the corresponding A/D input.  If broken for any reason, this input wont break threshold, inhibiting
  inverter start.
- Added Version Number to the Communication Bus: Index22, 23, 24.

Solar Microinverter 240V MOSFET Block 04-09-15 V1.01.07
- The current measurement was getting messed up with anti-islanding.  wCurrentACMax gets input in
  a current filter: wCurrFilter[] array and function InputFilterPoint().  Now, it doesn't affect 
  the measurement.

Solar Microinverter 240V MOSFET Block 03-27-15 V1.01.06
- Added Temperature Trip Point at 100 C
- Changed the PV Ground fault to look at the raw A/D value only to trip.
- Added Trip for VGrid L1-N and L2-N
- Fix the 60 hz +/- 2 Hz.  It is not a setpoint anymore
- Fixed the 5 Min timer for Grid Faults, including Freq.  It wasn't working right, added ZCDcount = 0;
  to the timer loop. Without it, inverter starts prematurely.
- Adjusted upper trip limit PV input voltage
- Had to take out if (bReceiveBuf[INDEX_SHT] != 0) since anti-islanding and timer disable info 
  wasn't being sent

Solar Microinverter 240V MOSFET Block 03-18-15 V1.01.04
- Adjusted PI contants.  Narrowed ZC window by changing ONEHUNDREDSEVENTYDEGREE from 31200 to 31900
	All this is done to improve THD at specific higher harmonics.  Put this code in the farm to see 
  if this tightening breaks inverters.

Solar Microinverter 240V MOSFET Block 03-18-15 V1.01.03
- Combined V1.01.02 with Charles V1.01.02 quad and singlet.  This project has THD of 4.5% at 1200W.
	Also, trottle up was done to 1.44KW output without cycling

Solar Microinverter 240V MOSFET Block 03-09-15 V1.01.02
- Added a mechanism to disable inverter for 5 Min after a grid fault.  This happens at beginning of main()
	This includes creating wGridFaultTimer and adding "Do Ingnore Grid Fault Timer" and "Disable Anti-Islanding"
  inside uart.c.  I had to move "Do Power Level" to the end so that Ingnore Grid and Disable Anti... would work

Solar Microinverter 220V MOSFET Block 03-04-15 1.01.00
- Vgrid a/d input changed from AN5 to AN7.  However, Vgrid is not being used in the code
- Vinv a/d input changed from AN3 to AN6. This input is being used all over the place, it is labeled VLL 
- Initialization for these 2 ADC was added to PVInverter.c
- This is all done to bring up new DV8 board for UL and betas

Solar Microinverter 220V MOSFET Block 02-27-15 1.00.14
- Programmed necessary things to get THD < 5% at power levels of 33%, 66% and 100%.
- Stopped cycling at PV > 40 Vdc by adjusting iRa2= Q15(.4991) from 0.9991

Solar Microinverter 220V MOSFET Block 01-27-15 1.00.13
- Fixed the accuracy of current measurement. Changed the current accuracy adjustement to fCurr /= 7.5;
	Added a case PROG_VARIABLE in uart.c to clear the error and error state remotely with a broadcast 
  from the Master Processor

Solar Microinverter 220V MOSFET Block 01-15-15 1.00.12
- When any error send inverter to case SYSTEM_ERROR: the inverter will execute TurnOffMOSFET_BRIDGE()
  immediately, which resets the dsPIC.

Solar Microinverter 220V MOSFET Block 12-21-14 1.00.11
- Deviced a manner to do setpoints and avoid resetting inverter by enable resetting 
  only after inverter has started producing power.  Doing this by using bSkip_SYSTEM_ERROR_Wdg_Reset
  variable.  This variable is set in main() but clear when setting power level in uart.c
	In case SYSTEM_ERROR: it avoids inverter reset when power setting is 1st set.		

Solar Microinverter 220V MOSFET Block 12-21-14 1.00.10
- Broke up the ulong variable into a word to try to avoid not able to start inverter
  since inverter sometimes got stock where it wouldn't start.  Also, avoided reseting
  inverter when SYSTEM_ERROR so it worked with setpoints.

Solar Microinverter 220V MOSFET Block 12-21-14 1.00.09
a. Added LED indicator code. LED code located in main and uart.c
b. Added PV input ground fault code.  It is located in main.  There is a define associated
   with this feature: DC_GROUND_FAULT  in order for this feature to work

Solar Microinverter 220V MOSFET Block 12-18-14 1.00.08
a. Merged Charles 4.8 THD code with the SetPoints code

Solar Microinverter 220V MOSFET Block 12-9-14 1.00.05
1- The units on the farm continued to experience hardware failures.  We discovered that the overcurrent protection was not working, and likely had not been working for awhile since the 
power of the unit was increased to 275/300W.  In order to properly detect overcurrent faults, we had to change the hardware scaling on the current sensing circuit.  In software,:
a. The overcurrent threshold was reduced.
b.  The MAX MPPT factor was also reduced.  
c.  A test piece of code tha
t commands an intentional short circuit was added.  This was used to test the software on the variac in the lab.

Solar Microinverter 220V MOSFET Block 12-3-14
1- After additional failures on the farm, we changed C51 to .01 uF to speed up current sensing.  In addition,:
	a. Reduced the proportional and integral gains. 
	b.  Removed Hardware ZC fault.  This was causing multiple spurious faults. This seemed to introduce an instability when operating 4 inverters in a quad.
	c. Added a software filter to the current input to the control loop.  The filter does not affect the overcurrent protection circuit.
  



Solar Microinverter 220V MOSFET Block 11-20-14
1- Changed several features after failures in the lab after changing C51 to .047 uF:
	a. Reduced the proportional gain.  This seemed to introduce an instability when operating 4 inverters in a quad.
	b. Changed ZeroCrossingDelay back to 20.
  c.  Changed ONEHUNDREDSEVENTYDEGREE to 31200 and ONEHUNDREDSEVENTYFIVEDEGREE to 31500.



Solar Microinverter 220V MOSFET Block 11-20-14
1- Changed several features to reduce current distortion to under 5% when operating from >34VDC input:
	a. Changed the proportional gain and the integral gain in the current regulator.  Increased proportional gain to .505 X2 = 1.1
	b. Removed the limiter for "TotalOutput" from the current regulator.  This variable is a long so it should not be limited to 32767.
	c.  Tightened the zero crossing window by redefining ONEHUNDREDSEVENTYDEGREE and ONEHUNDREDSEVENTYFIVEDEGREE.  Also lowered the zeroCrossDelay to 10 from 20.
	d.  Fixed a bug in finding the current and voltage offsets.  Previously, the offsets were fixed at startup.  Now, the offset is continuously calculated by averaging the AC waveform over 20 cycles.  
			The average is subtracted from the AD samples to obtain an offset.  This dramatically improved distortion.
	e.  Implemented a fix to the fault/shutdown procedure.  Now when a fault occurs the microcontroller is reset and re-initialized in PVInverter_Statemachine.c.  This keeps the inverter from breaking on startup.
	f.  Changed the gains on the balancing PI controller which uses the variables  MAX_BALANCE, KA_Q15 Q15, and KSA_Q15.  Increased gains by a factor of 10.

Solar Microinverter 220V MOSFET Block 11-04-14
1- Completely elimitated The Cycling and MOSFET Breakage Protection System formly known as 
	 Anti-islanding software.
2- Fix the polarity of OPTO_DRV1,2 inside TurnOffMOSFET_BRIDGE. This was reversed.  So when
	 shutting down inverter, MOSFET gates where basically left floating.  MOSFETs would blow.

Solar Microinverter 220V MOSFET Block 10-30-14
1- Opened up frequency window for 60 Hz +-3.1Hz
2- Tuned up The Cycling and MOSFET Breakage Protection System in: Threshold = 1800 (0.18V) 
   difference between deltaVac (synthezied sine x Vac Peak) and Vgrid. And 50 times threshold break.
   with this 2 numbers, there is minimun cycling on the farm and not able to break it in the lab with
   cycling power supplies/grid randomly

Solar Microinverter 220V MOSFET Block 10-12-14
Adjusted firmware so it looked identical than SCR design that doesn't cycle.  Took out low PV voltage
zero crossing scheme.  Commented out systemStartFlag = 2; where ever there was a fault. systemState =
SYSTEM_ERROR did this inside state machine case statement.  Increased many MAX values inside define.h

Solar Microinverter 220V MOSFET Block 10-07-14
Added a command to zero variables inside ParseCommand1().  Also, touched up many of the define limits in 
PVInverter_defines.h in order to try to fix the cycling.  BTW, cycling gets triggered when at the farm in 
POD C the generator voltage is above say 242Vac, connected to solar panel and mpptFactor > 8000 or approx 160Watts

Solar Microinverter 220V MOSFET Block 09-24-14
Adjusted the following in order to fix cycling:
Added:
	// At system Start-up, enable the Full-Bridge circuit (@ peak of AC Cycle) before the flyback circuit is enabled
	// If this is not done, the flyback output will have high DC voltage and when the full-bridge is enabled at the 
	// zero cross there is a large dv/dt and the output current will have a large glitch and also trip the flyback
	// OVP circuit 

	// ZCDcount>50 is 1/2 of above startup event (100)
	if (ZCDcount>50 && (nintyDegreeDetectFlag == 1))						// Set Flag to start full-bridge driveectFlag == 1)
	{
		startFullBridgeFlag = 1;		
		TestPin2 = 1;
	}
Also, needed to add: systemStartFlag to these 2 statements
			if (gridVoltage >= 200 && gridVoltage < 12000 && systemStartFlag == 0)				// 2000  --  12000
			if (gridVoltage < -200 && gridVoltage > -12000 && systemStartFlag == 0)				// -2000  --  -12000



Solar Microinverter 220V MOSFET Block 08-27-14
Changes done to fix THD:
1- Turn off MOSFETs @ ZC and back ON at brigde commutation to create a window around ZC
2- iRa changed  0.3621 ---> 1.0
3- Created averageRectifiedCurrent to shut off inverter in output overcurrent. Uncommneted systemState = SYSTEM_ERROR;
   inside AC overcurrent
4- Adjusted the MPPT factor increment/decrement		(left out)
5- Watchdog set to 2ms from 7ms
6- else if(systemRestartCounter > (10000*bAddress) ) inside State Machine (left out)
7- And last: added bRescueCounter in main.  Set it for 7 seconds software loop.  Test PV and Iac out,
   if in 7 Secs PV > 26V and Iac < 0.1 Amps we need to reset inverter.  Reason, there is discrepancy
   on spagetti code control system that inhibit inverters from starting up. This code is in main()

Added filtering to the AC current output sensor.  An 8 elemnet filter. Since these MOSFET units are
reinitializing and interering with each other.  So, lets start buy not reacting so quickly.  I need 
to put this on the Flyback current also, it hasn't been done. This hasn't been proven the cause of 
re-initialing or not starting.

Turn off MOSFETs before ZC to have a heathly gap at ZC.  To allow SCR or MOSFET bridge Turn off 
away from ZC, so to avoid mis-commutation, which is blowing SiC rectifiers. This was done in 
PVInverter_isr.c. Look for comment: Grid voltage going up and approaching ZC

Solar Microinverter 220V MOSFET Block 07-23-14
The commutating part of the inverter got changed to MOSFET bridge. Put commutating part on a function
called fullBridgeDrive(). THe initialization of PWM3 changed completely.  We are shutting down
immediately on a AC over current

PVMicroInverter 06-19-14
The Fix below helped resolve some major blowout problems.  However, the SiC diodes were still getting
blown.  2 things were changed.  
1- Put watchdog timer to 2 seconds and when there is a PV low voltage turn off MOSFETs and SCR in a
   continous basis in an infinite loop inside _ADCP0Interrupt() and wait for the watchdog to reset processor
2- This is what really fixed the problem: made #define ONEHUNDREDTENDEGREE			5000 which equals
	 1 ms instead of 5ms. This means that the SCR driving PWM3 pair is pulsed only 1ms instead of 5ms
   This allows the optotriac + SCR circuit to settle completely.  With a 5ms, the optotriacs were 
	 still being driven allowing gate of SCR to retrigger, commutating incorrectly into the other 
	 cycle of the grid, shorting the grid thru the SiC diodes, BAM!!! This problem took 3 weeks the 
   entire team to find.

PVMicroInverter 06-03-14
When there is a PV Unvervoltage the system would turn off the MOSFETs and SCR randomly.  In the LAB
with a solar panel we were getting Generator Faults when turning off MOSFETs/SCR randomly.  Now, in 
PVInverter_Statemachine.c at if (pvUnderVoltageCounter > 10), we don't shut off MOSFETs/SCR.  I 
arm bArmUnderVoltageShutDown. Inside PVInverter_isr.c, in case SCR_D4D5_ACTIVE:, case SCR_D3D6_ACTIVE:
which is 5ms after 0xsing, I start wUnderVoltageShutDownCounter = 2.8ms, so I shut down the MOSFETs
at 7.8ms or just before zero xsing inside _ADCP0Interrupt, so that shut down occurs at a point of near 
0 energy. The next instruction is a Software Reset!!!


PVMicroInverter 05-22-14
Inverter SiC diodes and other power MosFets breaking.  Adjusted the level and time at when the 
Inverter shuts off.  Also, decreased the time
pvUnderVoltageCounter from 400 to 20, that way we shut off faster, when PV goes below 23.5V setting
 

PVMicroInverter 05-14-14
Added PV voltage and Flayback peak currents 1 and 2 to intermodule packet

PVMicroInverter 05-01-14
Took out this code for suppressing instability.  Since the new transformer from Sumida came out, 
pn T91138, made system a lot more stable.  Added code to send PV voltage and PV Currents to master
processor, uart.c

PVMicroInverter 02-27-14
Added code to mitigate instability. It came down to adding boundaries to how big currentError
could grow.  currentError = IoRef - rectifiedIacQ15 which is the current output.  This 
would get unstable at > 200 Watts at the crest of sinewave.  By limiting to CURRENT_ERROR_LIMIT
the unstability wouldnt get put back into the plant anymore, therefore making the current output
more stable at crests of sinewave. Also, #define SUPPRESS_MPPT code needs to get deleted.  It is
no good.  #define WAVE_ANALYSIS is good code to output arrays of data and specific variables


PVMicroInverter 11-22-13
Read address of slave from resistor network on RB7 and RB6. Created function to do this:
unsigned char	ReadResistorsForAddr(void);

RB7  RB6 | Inverter Addr
-------------------------
 0    0  |     2
 0    1  |     3
 1    0  |     4
 1    1  |     5

PVMicroInverter 06-03-13
Changed the POR from 128ms to 4ms.  When big shadow on the solar panel firmware would crash.


PVMicroInverter 10-17-12
Added uart.c and uart.h communication protocol.  Tested it with Inverter Display.

****************************************************************************************************/