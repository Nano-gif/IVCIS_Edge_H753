# Introduction

The STM32H7 Nucleo-144 boards, based on the MB1364 reference board (NUCLEO-H723ZG, NUCLEO-H743ZI (order code NUCLEO-H743ZI2), and NUCLEO-H753ZI), provide an affordable and flexible way for users to try out new concepts and build prototypes, by choosing from the various combinations of performance and power consumption features provided by the STM32H7 series microcontroller. The ST Zio connector, which extends the ARDUINO® Uno V3 connectivity, and the ST morpho headers provide an easy means of expanding the functionality of the Nucleo open development platform with a wide choice of specialized shields. The STM32H7 Nucleo-144 boards do not require any separate probe as they integrate the STLINK-V3E debugger/programmer. The STM32H7 Nucleo-144 boards come with comprehensive free software libraries and examples available with the STM32Cube MCU Package.

Note: For NUCLEO-H743ZI (STM32H7 Nucleo-144 (MB1137) - order code NUCLEO-H743ZI), refer to UM1974.


Figure 1. Nucleo-144 board (top view)


![](images/5c16666bf41986ec45784e35266dc537fd4b40e43505f442245be3a94a759e12.jpg)



Figure 2. Nucleo-144 board (bottom view)


![](images/ed68957272265f9f42ae16e1d12091db2b5df6c44481674d18e329e58cf06d93.jpg)



Pictures are not contractual.


# Contents

1 Features 7

2 Ordering information 8

2.1 Codification 8

3 Development environment 9

3.1 System requirements 9

3.2 Development toolchains 9

3.3 Demonstration software 9

3.4 EDA resources 9

4 Conventions 10

5 Safety recommendations 11

5.1 Targeted audience 11

5.2 Handling the board 11

6 Quick start 12

6.1 Getting started 12

7 Hardware layout and configuration 13

7.1 Nucleo-144 board layout 14

7.2 Mechanical drawing 16

7.3 EmbeddedSTLINK-V3E 18

7.3.1 Drivers 18

7.3.2 STLINK-V3E firmware upgrade 19

7.3.3 Using an external debug tool to program and debug the on-board STM32H7 19

7.4 Power supply 21

7.4.1 Power supply input from STLINK-V3E USB connector (default setting) 21

7.4.2 External power supply input from VIN (7 to 12 V, 800 mA max) 22

7.4.3 External power supply input 5V_EXT (5 V, 500 mA max) 23

7.4.4 External power supply input from a USB charger (5 V) 24

7.4.5 External power supply input from 3V3_EXT (3.3 V) 24

7.4.6 Debugging while using VIN or EXT as an external power supply 25

7.5Clock sources 26

7.5.1 HSE clock (high-speed external clock) 26

7.5.2 LSE clock (low-speed external clock) - 32.768 kHz 27

7.6 Board functions 27

7.6.1 LEDs 27

7.6.2 Push Buttons 28

7.6.3 MCU voltage selection: 1V8/3V3 28

7.6.4 Current consumption measurement (IDD) 28

7.6.5 Virtual COM port (VCP): LPUART/USART 28

7.6.6 USBOTG_FS 29

7.6.7 Ethernet 30

7.7 Solder bridges and jumpers 31

8 Board connectors 36

8.1 STLINK-V3E USB Micro-B connector (CN1) 36

8.2 USB OTG_FS connector (CN13) 36

8.3 Ethernet RJ45 connector (CN14) 37

9 Extension connectors 38

9.1 ST Zio connectors 38

9.2 ST morpho connector 43

10 Nucleo-144 boards (MB1364) information 45

10.1 Product marking 45

10.2 Nucleo-144 boards (MB1364) product history 46

10.3 Board revision history 48

11 Compliance statements and conformity declarations 49

11.1 Federal Communications Commission (FCC) compliance statement 49

11.2 Innovation, Science and Economic Development Canada (ISED) compliance statement 50

11.3 UKCA conformity 50

11.4 CE conformity 50

11.4.1 Simplified EU declaration of conformity 50

11.4.2 Déclaration de conformité UE simplifiée 50

12 Product disposal 52

Revision history 53

# List of tables

Table 1. Ordering information.. 8

Table 2. Codification explanation 8

Table 3. ON/OFF conventions 10

Table 4. Jumper configuration 12

Table 5. MIPI-10 debug connector (CN5). 20

Table 6. External power sources: VIN (7 to 12 V) 23

Table 7. External power sources: 5V_EXT 23

Table 8. External power sources: CHGR (5 V). 24

Table 9. External power sources: 3V3_EXT (3.3 V). 25

Table 10.USART3 connection 29

Table 11. LPUART1 connection 29

Table 12. USB pin configuration 30

Table 13. Ethernet pin configuration 30

Table 14. Solder bridge and jumper configuration 31

Table 15. USB Micro-B connector (CN1) pinout. 36

Table 16. USB OTG_FS Micro-AB connector (CN13) pinout. 36

Table 17. Ethernet RJ45 connector (CN14) pinout. 37

Table 18. ZIO connector (CN7) pinout 40

Table 19. ZIO connector (CN8) pinout 40

Table 20. ZIO connector (CN9) pinout 41

Table 21. ZIO connector (CN10) pinout 42

Table 22. Pin assignment of the ST morpho connector 43

Table 23. Product history 46

Table 24. Board revision history 48

Table 25. Document revision history 53

# List of figures

Figure 1. Nucleo-144 board (top view).

Figure 2. Nucleo-144 board (bottom view). 1

Figure 3. Hardware block diagram. 13

Figure 4. Nucleo-144 board top layout. 14

Figure 5. Nucleo-144 bottom layout. 15

Figure 6. Nucleo-144 board mechanical drawing in millimeters. 16

Figure 7. Nucleo-144 board mechanical drawing in mils 17

Figure 8. USB composite device 19

Figure 9. Connecting an external debug tool to program the on-board STM32H7 20

Figure 10. Power supply input from STLINK-V3E USB connector with PC (5 V, 500 mA max) . . . . . 22

Figure 11. Power supply input from VIN (7 to  $12\mathrm{V}$ $800\mathrm{mA}$  max) 23

Figure 12. Power supply input from 5V_EXT (5 V, 500 mA max) 24

Figure 13. Power supply input from STLINK-V3E USB connector with a USB charger (5 V) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

Figure 14. Power supply input from 3V3_EXT (3.3 V). 25

Figure 15. Ethernet RJ45 connector (CN14) front view. 37

Figure 16. NUCLEOH7 Nucleo-144 board. 38

# 1 Features

The STM32H7 Nucleo-144 boards offer the following features:

- STM32H7 Arm®(a) Cortex® core-based microcontroller in an LQFP144 package

- Ethernet compliant with IEEE-802.3-2002 (depending on STM32H7 support)

USB OTG full-speed

3 user LEDs

- 2 push Buttons: USER and RESET

LSE crystal:

32.768 kHz crystal oscillator

Board connectors:

USB with Micro-AB

- Ethernet RJ45

MIPI-10

ST Zio including ARDUINO® Uno V3 expansion connectors

ST morpho expansion connector

- Flexible power supply options: ST-LINK USB  $V_{BUS}$  or external sources

- On-board STLINK-V3E debugger/programmer with SWD connector:

- USB reenumeration capability: Virtual COM port, mass storage, debug port

- STLINK-V3E standalone kit capability

- Comprehensive free software libraries and examples available with the STM32Cube package

- Support of a wide choice of Integrated Development Environments (IDEs) including IAR Embedded Workbench®, MDK-ARM, and STM32CubeIDE

arm

# 2 Ordering information

To order the Nucleo-144 board corresponding to the targeted STM32, use the order code given below in Table 1:


Table 1. Ordering information


<table><tr><td>Order code</td><td>Board reference</td><td>Target STM32H7</td><td>Differentiating feature</td></tr><tr><td>NUCLEO-H723ZG</td><td rowspan="3">MB1364(1)</td><td>STM32H723ZGT6</td><td>-</td></tr><tr><td>NUCLEO-H743ZI2</td><td>STM32H743ZIT6</td><td>-</td></tr><tr><td>NUCLEO-H753ZI</td><td>STM32H753ZIT6</td><td>Cryptography</td></tr></table>


1. Subsequently named main board in the rest of the document.


# 2.1 Codification

The meaning of the codification is explained in Table 2.


Table 2. Codification explanation


<table><tr><td>NUCLEO-XXYYZTN</td><td>Description</td><td>Example: NUCLEO-H743ZI2</td></tr><tr><td>XX</td><td>MCU series in STM32 32-bit Arm Cortex MCUs</td><td>STM32H7 series</td></tr><tr><td>YY</td><td>MCU product line in the series</td><td>STM32H743</td></tr><tr><td>Z</td><td>STM32 package pin count</td><td>144 pins</td></tr><tr><td>T</td><td>STM32H7 flash memory size:
-G for 1 Mbyte
-I for 2 Mbytes</td><td>2 Mbytes</td></tr><tr><td>N</td><td>Board version: void or 2</td><td>STLINK-V3E</td></tr></table>

# 3 Development environment

# 3.1 System requirements

- Multi-OS support: Windows® 10 or 11, Linux® 64-bit, or macOS®(a)(b)(c)

- USB Type-A or USB Type-C® to Micro-B cable

# 3.2 Development toolchains

- IAR Systems® - IAR Embedded Workbench®(d)

- Keil® - MDK-ARM(d)

STMicroelectronics - STM32CubeIDE

# 3.3 Demonstration software

The demonstration software, included in the STM32Cube MCU Package corresponding to the on-board microcontroller, is preloaded in the STM32 flash memory for easy demonstration of the device peripherals in standalone mode. The latest versions of the demonstration source code and associated documentation can be downloaded from www.st.com.

# 3.4 EDA resources

All board design resources, including schematics, EDA databases, manufacturing files, and the bill of materials are available from the relevant product page at www.st.com.

# 4 Conventions


Table 3 provides the conventions used for the ON and OFF settings in the present document.



Table 3. ON/OFF conventions


<table><tr><td>Convention</td><td>Definition</td></tr><tr><td>Jumper JPx ON</td><td>Jumper fitted</td></tr><tr><td>Jumper JPx OFF</td><td>Jumper not fitted</td></tr><tr><td>Jumper JPx [1-2]</td><td>Jumper fitted between pin 1 and pin 2</td></tr><tr><td>Solder bridge SBx ON</td><td>SBx connections closed by 0 Ω resistor</td></tr><tr><td>Solder bridge SBx OFF</td><td>SBx connections left open</td></tr><tr><td>Resistor Rx ON</td><td>Resistor soldered</td></tr><tr><td>Resistor Rx OFF</td><td>Resistor not soldered</td></tr><tr><td>Capacitor Cx ON</td><td>Capacitor soldered</td></tr><tr><td>Capacitor Cx OFF</td><td>Capacitor not soldered</td></tr></table>

In this document, for any information that is common to all sales types, the references are noted as the STM32H7 Nucleo-144 board and STM32H7 Nucleo-144 boards.

# 5 Safety recommendations

# 5.1 Targeted audience

This product targets users with at least basic electronics or embedded software development knowledge such as engineers, technicians, or students. This board is not a toy and is not suited for use by children.

# 5.2 Handling the board

This product contains a bare printed circuit board and like all products of this type, the user must be careful about the following points:

- The connection pins on the board might be sharp. Be careful when handling the board to avoid hurting yourself.

This board contains static-sensitive devices. To avoid damaging it, handle the board in an ESD-proof environment.

While powered, do not touch the electric connections on the board with your fingers or anything conductive. The board operates at a voltage level that is not dangerous, but components might be damaged when shorted.

- Do not put any liquid on the board and avoid operating the board close to water or at a high humidity level.

- Do not operate the board if dirty or dusty.

# 6 Quick start

The STM32H7 Nucleo-144 board is a low-cost and easy-to-use development kit, used to evaluate and start development quickly with an STM32H7 series microcontroller in an LQFP144 package.

Before installing and using the product, accept the evaluation product license agreement (EPLA) from the www.st.com/epla web page. For more information on the STM32H7 Nucleo-144 and demonstration software, visit the www.st.com stm32nucleo web page.

# 6.1 Getting started

Follow the sequence below to configure the Nucleo-144 board and launch the demonstration application (for component location, refer to Figure 4):

1. Check the jumper position on the board:


Table 4. Jumper configuration


<table><tr><td>Jumper</td><td>Definition</td><td>Position</td><td>Comment</td></tr><tr><td>JP1</td><td>STLK_RST</td><td>OFF</td><td>-</td></tr><tr><td>JP2</td><td>Power source selection</td><td>ON [1-2]</td><td>5V_USB_STLK (from ST-LINK)</td></tr><tr><td>JP3</td><td>T_NRST</td><td>ON</td><td>-</td></tr><tr><td>JP4</td><td>IDD measurement</td><td>ON</td><td>MCU current measurement</td></tr><tr><td rowspan="2">JP5</td><td rowspan="2">VDD MCU power selection</td><td>ON [1-2] (default)</td><td>VDD MCU supplied with 3V3_VDD</td></tr><tr><td>ON [2-3] (optional)</td><td>VDD MCU supplied with 1V8_VDD</td></tr></table>

2. For the correct identification of the device interfaces from the host PC and before connecting the board, install the Nucleo USB driver available on the www.st.com stm32nucleo website.

3. Power the board by connecting the STM32H7 Nucleo-144 board to a PC with a USB Type-A to Micro-B cable through the USB connector (CN1) on the ST-LINK. As a result, the PWR green LED (LD5) and COM LED (LD4) light up and the red LED (LD3) blinks.

4. Press B1 (left button).

5. Observe the blinking frequency of the three LEDs LD1 to LD3 changes, by clicking on the B1 button.

6. The software demonstration and the several software examples that allow the user to use the Nucleo features, are available at the www.st.com stm32nucleo web page.

7. Develop an application, using the available examples.

# 7 Hardware layout and configuration

The STM32H7 Nucleo-144 board is designed around the STM32H7 series microcontrollers in a 144-pin LQFP package.

Figure 3 shows the connections between the STM32H7 and its peripherals (STLINK-V3E, push Buttons, LEDs, USB, Ethernet, ST Zio connectors, and ST morpho headers).

Figure 4 and Figure 5 show the location of these features on the STM32H7 Nucleo-144 board.

The mechanical dimensions of the board are shown in Figure 6 and Figure 7.


Figure 3. Hardware block diagram


![](images/edf8b5090344c3e08e71fe539aeb4f08b7f0a78eccbee7a135301c35e4c4885b.jpg)



MSv51396V1


# 7.1 Nucleo-144 board layout


Figure 4. Nucleo-144 board top layout


![](images/4fde2f832aa67c66abd9b6f4efdd257b030f077855bc1e9cc90eb2a1760e72cf.jpg)



Figure 5. Nucleo-144 bottom layout


![](images/00d7a4c568487a845b6768f8347255868b4e48dc074c062d7671c32d833c5097.jpg)


IOREF Power selection (SB10, SB11, and SB20)

MSv51398V3

# 7.2 Mechanical drawing


Figure 6. Nucleo-144 board mechanical drawing in millimeters


![](images/bd34b40e86b3e4dc4095f73b40eccb7e8c935a69cc504bcc239440ac6e70cd91.jpg)



Figure 7. Nucleo-144 board mechanical drawing in mils


![](images/a4d56061b4627e6faba06c5600ba9faa7e467e51c030b767583273151571ff10.jpg)


# 7.3 Embedded STLINK-V3E

There are two different ways to program or debug the on-board STM32H7 MCU:

Using the embedded STLINK-V3E

- Using an external debug tool connected to the MIPI-10 connector (CN5).

The STLINK-V3E programming and debugging tool is integrated into the STM32H7 Nucleo-144 board.

The embedded STLINK-V3E supports only SWD and VCP for STM32H7 devices. For information about debugging and programming features, refer to the technical note Overview of ST-LINK derivatives (TN1235), which describes in detail all the STLINK/V3 features.

Features supported on STLINK-V3E:

- 5 V power supplied by a USB connector (CN1)

USB 2.0 high-speed-compatible interface

- JTAG/serial wire debugging (SWD) specific features:

3.0 to 3.6 V application voltage on the JTAG/SWD interface and 5 V-tolerant inputs

- JTAG

SWD and serial viewer (SWV) communication

STDC14 (MIPI-10) compatible connector (CN5)

Status COM LED (LD4) which blinks during communication with the PC

- Fault red LED (LD6) alerting on USB overcurrent request

- 5 V/300 mA output power supply capability (U2) with current limitation and LED

- Green LED ON: 5 V enabled (LD5)

# 7.3.1 Drivers

Before connecting the Nucleo-144 board to a Windows® PC via USB, a driver for STLINK-V3E must be installed (not required for Windows® 10). The toolset supporting ST-LINK automatically installs the driver. It is also available from the www.st.com website.

In case the STM32H7 Nucleo-144 board is connected to the PC before installing the driver, the PC device manager might report some Nucleo interfaces as Unknown.

To recover from this situation, after installing the dedicated driver, the association of Unknown USB devices found on the STM32H7 Nucleo-144 board to this dedicated driver must be updated in the device manager manually.

Note: ST recommends the use of a USB composite device, as shown in Figure 8.


Figure 8. USB composite device


![](images/2826f233e3eacb848e04b63039af386e792a8d31b612396f045d969f98b3fc9a.jpg)



Note: 37xx = 374E for STLINK-V3E without bridge functions 374F for STLINK-V3E with bridge functions


# 7.3.2 STLINK-V3E firmware upgrade

The STLINK-V3E embeds a firmware upgrade mechanism for an in-place upgrade through the USB port. As the firmware might evolve during the lifetime of the STLINK-V3E product (for example new functionalities, bug fixes, and support for new microcontroller families), ST recommends keeping the STLINK-V3E firmware up to date before starting to use the STM32H7 Nucleo-144 board. The latest version of this firmware is available from the www.st.com website.

# 7.3.3 Using an external debug tool to program and debug the on-board STM32H7

There are two basic ways to support an external debug tool:

1. Keep the embedded STLINK-V3E running. Power on the STLINK-V3E at first until the COM LED lights RED. Then connect your external debug tool through the MIPI-10 debug connector (CN5).

2. Set the embedded STLINK-V3E in the high-impedance state: When the STLK_RST jumper (JP1) is ON, the embedded STLINK-V3E is in the RESET state, and all GPIOs are in high impedance. Then it is possible to connect the external debug tool to the debug connector (CN5).


Figure 9. Connecting an external debug tool to program the on-board STM32H7


![](images/8eb12cef143a001a278c8717b43307f64e1030048a1132c8ee1c9642a0e15b00.jpg)



Table 5. MIPI-10 debug connector (CN5)


<table><tr><td>MIPI-10 Pin</td><td>STDC14 Pin</td><td>CN5</td><td>Designation</td></tr><tr><td>-</td><td>1</td><td>NC</td><td>Reserved</td></tr><tr><td>-</td><td>2</td><td>NC</td><td>Reserved</td></tr><tr><td>1</td><td>3</td><td>T_VCC</td><td>Target VCC</td></tr><tr><td>2</td><td>4</td><td>T_SWDIO</td><td>Target SWDIO using SWD protocol or target JTMS (T_JTMS) using JTAG protocol</td></tr><tr><td>3</td><td>5</td><td>GND</td><td>Ground</td></tr><tr><td>4</td><td>6</td><td>T_SWCLK</td><td>Target SWCLK using SWD protocol or target JCLK (T_JCLK) using JTAG protocol</td></tr><tr><td>5</td><td>7</td><td>GND</td><td>Ground</td></tr><tr><td>6</td><td>8</td><td>T_SWO</td><td>Target SWO using SWD protocol or target JTDO (T_JTMS) using JTAG protocol</td></tr><tr><td>7</td><td>9</td><td>T_JRCLK</td><td>Not used by SWD protocol, target JRCLK (T_JRCLK) using JTAG protocol, only for specific use</td></tr><tr><td>8</td><td>10</td><td>T_JTDI</td><td>Not used by SWD protocol, target JTDI (T_JTDI) using JTAG protocol, only for external tools</td></tr><tr><td>9</td><td>11</td><td>GNDDetect</td><td>GND detection signal for plug indicator, used on SWD and JTAG neither</td></tr><tr><td>10</td><td>12</td><td>T_NRST</td><td>Target NRST using SWD protocol or target JTMS (T_JTMS) using JTAG protocol</td></tr></table>


Table 5. MIPI-10 debug connector (CN5) (continued)


<table><tr><td>MIPI-10 Pin</td><td>STDC14 Pin</td><td>CN5</td><td>Designation</td></tr><tr><td>-</td><td>13</td><td>T_VCP_RX</td><td>Target RX used for VCP (must be UART dedicated to bootloader)</td></tr><tr><td>-</td><td>14</td><td>T_VCP_TX</td><td>Target TX used for VCP (must be UART dedicated to bootloader)</td></tr></table>

# 7.4 Power supply

Five different sources can provide the power supply:

- A host PC connected to CN1 through a USB cable (default setting)

An external 7 to  $12\mathrm{V}$  power supply connected to CN8 pin 15 or CN11 pin 24

- An external 5 V power supply connected to CN11 pin 6

- An external 5 V USB charger (5V_USB_CHGR) connected to CN1

- An external  $3.3 \mathrm{~V}$  power supply (3V3) connected to CN8 pin 7 or CN11 pin 16

Either the host PC through the USB cable, or an external source  $V_{\mathrm{IN}}$  (7 to 12 V), E5V (5 V), or +3.3 V power supply pins on CN8 or CN11, provides the power supply. In case  $V_{\mathrm{IN}}$ , E5V, or +3.3 V is used to power the Nucleo-144 board.

Note: In case the power supply is +3.3 V, the STLINK-V3E is not powered and cannot be used. The STM32H7 Nucleo-144 board must be powered by a power supply unit, or by auxiliary equipment complying with the EN 62368-1:2014+A11:2017 standard. It must be a safety extralow voltage (SELV) with limited power capability.

# 7.4.1 Power supply input from STLINK-V3E USB connector (default setting)

The 5 V signal on the STLINK-V3E USB connector (CN1) can power the STM32H7 Nucleo-144 board and its shield. Use the JP2 [1-2] configuration of the STLINK jumper (refer to Figure 10).

This is the default setting.


Figure 10. Power supply input from STLINK-V3E USB connector with PC (5 V, 500 mA max)


![](images/d62c418e1dd17138f6f21bde9b4f6874c4ae801e886d49034d514216ad6670fc.jpg)



MSv61203V2


If the USB enumeration succeeds, the 5V_STLINK power is enabled, by asserting the PWR_ENn signal from STM32F723IEK6 'STLINK-V3E' (U7). This pin is connected to a power switch (U2), which powers the board. The power switch (U2) features also a current limitation to protect the PC in case of a short-circuit onboard. If an overcurrent (more than 500 mA) happens onboard, the red LED (LD6) is lit.

The STLINK-V3E USB connector (CN1) can power the Nucleo board with its shield. However, the STLINK-V3E circuit gets power before USB enumeration because the host PC only provides  $100\mathrm{mA}$  to the board at that time.

During the USB enumeration, the Nucleo board asks for  $500\mathrm{mA}$  power to the host PC.

- If the host can provide the required power, the enumeration finishes with a SetConfiguration command. Then, the power switch and the green LED (LD5) are turned ON. Thus, the Nucleo board and its shield can consume 500 mA current, but no more.

- If the host is not able to provide the requested current, the enumeration fails. Therefore, the power switch (U2) remains OFF and the MCU part including the extension board is not powered. As a consequence, the green LED (LD5) remains turned OFF. In this case, it is mandatory to use an external power supply.

Warning: In case the maximum current consumption of the STM32H7 Nucleo-144 board and its shield boards exceed  $300\mathrm{mA}$ , it is mandatory to power the STM32H7 Nucleo-144 board, using an external power supply connected to E5V,  $\mathsf{V}_{\mathsf{IN}}$ , or +3.3 V.

# 7.4.2 External power supply input from VIN (7 to 12 V, 800 mA max)

When the STM32H7 Nucleo-144 board is power supplied by VIN (refer to Table 6 and Figure 11), the jumper (JP2) configuration must be [3-4] VIN.

The Nucleo-144 board and its shield boards can be powered in three different ways from an external power supply, depending on the voltage used. The three power sources are summarized in Table 6.


Table 6. External power sources: VIN (7 to 12 V)


<table><tr><td>Input power name</td><td>Connector pins</td><td>Voltage range</td><td>Max current</td><td>Limitation</td></tr><tr><td>VIN</td><td>CN8 pin 15
CN11 pin 24</td><td>7 to 12 V</td><td>800 mA</td><td>From 7 to 12 V only and input current capability is linked to input voltage:
800 mA input current when VIN=7 V
450 mA input current when 7 V&lt;VIN&lt;9 V
250 mA input current when 9 V&lt;VIN&lt;12 V</td></tr></table>


Figure 11. Power supply input from VIN (7 to  $12\mathrm{V}$ , 800 mA max)


![](images/92a3327008a0cb892515592ff38cfdebef1961e0c4d75cf684bc37af268e530a.jpg)



MSv61204V2


Note: Refer to Section 7.4.6 about debugging when using an external power supply.

# 7.4.3 External power supply input 5V_EXT (5 V, 500 mA max)

When the STM32H7 Nucleo-144 board is power supplied by EXT (refer to Table 7 and Figure 12), the jumper configuration must be the following: JP2 jumper on pin 5-6 'EXT'


Table 7. External power sources: 5V_EXT


<table><tr><td>Input power name</td><td>Connector pins</td><td>Voltage range</td><td>Max current</td></tr><tr><td>EXT</td><td>CN11 pin 6</td><td>4.75 V to 5.25 V</td><td>500 mA</td></tr></table>

Note: Refer to Section 7.4.6 about debugging when using an external power supply.


Figure 12. Power supply input from 5V_EXTERNAL (5 V, 500 mA max)


![](images/33cbcf68770a116fb64e33a9e1e6fa870924738b1e95a170679e03fe293dba56.jpg)



MSv61205V2


# 7.4.4 External power supply input from a USB charger (5 V)

When the STM32H7 Nucleo-144 board is power supplied by a USB charger on CN1 (refer to Table 8 and Table 13), the jumper configuration must be JP2 [7-8] CHGR.


Table 8. External power sources: CHGR (5 V)


<table><tr><td>Input power name</td><td>Connector pins</td><td>Voltage range</td><td>Max current</td></tr><tr><td>CHGR</td><td>CN1</td><td>5 V</td><td>-</td></tr></table>


Figure 13. Power supply input from STLINK-V3E USB connector with a USB charger (5 V)


![](images/1b8f08e3e08caeb33f41278140abb3455caea0ac401f2901e76cbfd186d1ef02.jpg)



MSv61206V3


# 7.4.5 External power supply input from 3V3_EXT (3.3 V)

When the  $3.3\mathrm{V}$  is provided by a shield board, it is worthy to use the 3V3 (CN8 pin 7 or CN11 pin 16) directly as power input (refer to Table 9 and Figure 14). In this case, programming and debugging features are not available, since the STLINK-V3E is not powered.


Table 9. External power sources: 3V3_EXT (3.3 V)


<table><tr><td>Input power name</td><td>Connector pins</td><td>Voltage range</td><td>Max current</td></tr><tr><td>3V3</td><td>CN8 pin 7
CN11 pin 16</td><td>3 V to 3.6 V</td><td>1.3 A</td></tr></table>


Figure 14. Power supply input from 3V3_EXT (3.3 V)


![](images/3bc2e3295aacfb2f35f36dc19adab821221ccd5641b40e1a15a2e9703752108b.jpg)



MSv61207V2


# 7.4.6 Debugging while using VIN or EXT as an external power supply

When powered by VIN or EXT, it is still possible to use the STLINK-V3E for programming or debugging only. In this case, it is mandatory to power the board first using VIN or EXT, then to connect the USB cable to the PC. In this way, the enumeration succeeds, thanks to the external power source.

The following power-sequence procedure must be respected:

1. Configure the JP2 jumper [5-6] for EXT or [3-4] for VIN.

2. Connect the external power source to VIN or EXT.

3. Power on the external power supply  $7\mathrm{V} < \mathrm{VIN} < 12\mathrm{V}$  to VIN, or  $5\mathrm{V}$  for EXT.

4. Check that the green LED (LD5) is turned ON.

5. Connect the PC to the USB connector (CN1).

If this order is not respected,  $V_{BUS}$  coming from STLINK-V3E might power the board, and the following risks might be encountered:

1. If the board needs more than  $300\mathrm{mA}$  current, the PC might be damaged, or the PC can limit the supplied current. As a consequence, the board is not powered correctly.

2.  $300\mathrm{mA}$  is requested during enumeration. So, there is a risk that the request is rejected and the enumeration does not succeed if the PC cannot provide such current. Consequently, the board is not power-supplied. The green LED (LD5) remains OFF.

# 7.5 Clock sources

# 7.5.1 HSE clock (high-speed external clock)

There are four ways to configure the pins corresponding to the external high-speed clock (HSE):

- MCO from ST-LINK (Default): the MCO output of ST-LINK is used as an input clock. By default, it is fixed at 8 MHz and connected to PF0/PH0-OSC_IN of the STM32H7 series microcontroller. The frequency may be changed during ST-Link firmware upgrade (for more details, refer to RN0093, available from www.st.com).

SB44 and SB46 OFF

SB45 ON

SB3 and SB4 OFF

- HSE on-board oscillator from X3 crystal (not provided): For its typical frequencies, capacitors, and resistors, refer to the STM32H7 series microcontroller datasheet and the application note Oscillator design guide for STM8AF/AL/S and STM32 microcontrollers (AN2867) for the oscillator design guide. The X3 crystal has the following characteristics: 25 MHz, 6 pF, 20 ppm. The configuration must be:

SB44 and SB46 OFF

SB3 and SB4 ON

C69 and C70 ON with 5.6 pF capacitors

SB45 OFF

- Oscillator from external PF0/PH0: from an external oscillator through pin 29 of the CN11 connector. The configuration must be:

SB46 ON

SB45 OFF

SB3 and SB4 OFF

- HSE not used: PF0/PH0 and PF1/PH1 are used as GPIOs instead of clocks. The configuration must be:

SB44 and SB46 ON

SB45 OFF

SB3 and SB4 OFF

# 7.5.2 LSE clock (low-speed external clock) - 32.768 kHz

There are three ways to configure the pins corresponding to the low-speed clock (LSE):

- On-board oscillator (default): X2 crystal. Refer to the application note Oscillator design guide for STM8AF/AL/S and STM32 microcontrollers (AN2867) for oscillator design guide for STM32H7 series microcontrollers. The configuration must be:

SB40 and SB41 OFF

R38 and R39 ON

- Oscillator from external PC14: From an external oscillator through pin 25 of the CN11 connector. The configuration must be:

SB40 and SB41 ON

R38 and R39 OFF

- LSE not used: PC14 and PC15 are used as GPIOs instead of the low-speed clock. The configuration must be:

SB40 and SB41 ON

R38 and R39 OFF

# 7.6 Board functions

# 7.6.1 LEDs

User LD1: A green user LED is connected to the STM32H7 I/O PB0 (SB39 ON and SB47 OFF) or PA5 (SB47 ON and SB39 OFF) corresponding to the ST Zio D13.

User LD2: A yellow user LED is connected to PE1.

User LD3: A red user LED is connected to PB14.

These user LEDs are on when the I/O is HIGH value, and are off when the I/O is LOW.

COM LD4: The tricolor (green, orange, and red) LED (LD4) provides information about STLINK communication status. LD4 default color is red. LD4 turns to green to indicate that the communication is in progress between the PC and the STLINK-V3E, with the following setup:

- Slow blinking red/OFF at power-on before USB initialization

- Fast blinking red/OFF after the first correct communication between the PC and STLINK-V3E (enumeration)

- Red LED ON when the initialization between the PC and STLINK-V3E is complete

- Green LED ON after a successful target communication initialization

- Blinking red/green during communication with the target

Green ON communication finished and successful

Orange ON communication failure

PWR LD5: The green LED (LD5) indicates that the STM32H7 part is powered. The  $+5\mathrm{V}$  power is available on CN8 pin 9 and CN11 pin 18.

USB power fault LD6: The red LED (LD6) indicates that the board power consumption on USB exceeds  $500\mathrm{mA}$ , consequently, the user must power the board using an external power supply.

USB FS LD7 and LD8: Refer to USB OTG_FS.

# 7.6.2 Push Buttons

B1 USER (blue button): the user button is connected to the I/O PC13 by default (tamper support: SB51 ON and SB58 OFF) or PA0 (wake-up support: SB58 ON and SB51 OFF) of the STM32H7 series microcontroller.

B2 RESET (black button): this push-button is connected to NRST and is used to reset the STM32H7 series microcontroller.

# 7.6.3 MCU voltage selection: 1V8/3V3

The STM32H7 Nucleo-144 board offers the possibility to supply the STM32H7 series microcontroller with 1.8 V or 3.3 V. JP5 is used to select the VDD MCU power level.

- Place the JP5 jumper on 3V3 to supply the MCU with 3V3, connecting pins 1 and 2.

- Place the JP5 jumper on 1V8 to supply the MCU with 1V8, connecting pins 2 and 3.

# 7.6.4 Current consumption measurement (IDD)

The IDD jumper (JP4) is used to measure the STM32H7 series microcontroller consumption by removing the jumper and connecting an ammeter:

JP4 must be ON when STM32H7 is powered with 3V3_VDD (default)

- If JP4 is OFF, an ammeter must be connected to measure the STM32H7 current. If there is no ammeter, the STM32H7 is not powered.

Warning: On MB1364 REV.C, 'VDD MCU' is also supplying Ethernet PHY (U15) and debug voltage translation (U1 and U10).

If needed, for low power measurement (for example Standby mode), to measure only MCU (U7) power consumption, the user must remove the following components: R4, R43, R44, R45, R46, R47, R48, R49, R50, R51, R52, R53, R59, R61, U1, U10, U15, and SB45.

After removing these components, it becomes impossible to use Ethernet and 1.8 V debug with STLINK-V3E.

# 7.6.5 Virtual COM port (VCP): LPUART/USART

The STM32H7 Nucleo-144 board enables connecting an LPUART or aUSART interface to the STLINK-V3E, or to the ST morpho and ARDUINO Uno V3 connectors.

The selection is done by setting the related solder bridges (refer to Table 10 and Table 11 below).

By default theUSART3 communication between the target STM32H7 and the STLINK-V3E is enabled, to support the Virtual COM port (SB12 and SB19 ON).


Table 10.USART3 connection


<table><tr><td>Pin name</td><td>Function</td><td>Virtual COM port (default configuration)</td><td>ST morpho connection</td></tr><tr><td rowspan="2">PD8</td><td rowspan="2">USART3 TX</td><td>SB19 ON</td><td>SB82 ON</td></tr><tr><td>SB9, SB18, and SB82 OFF</td><td>SB18 and SB19 OFF</td></tr><tr><td rowspan="2">PD9</td><td rowspan="2">USART3 RX</td><td>SB12 ON</td><td>SB81 ON</td></tr><tr><td>SB34, SB66, and SB81 OFF</td><td>SB12 and SB66 OFF</td></tr></table>


Table 11. LPUART1 connection


<table><tr><td>Pin name</td><td>Function</td><td>Virtual COM port</td><td>ARDUINO® D0 and D1</td><td>ST morpho connection</td></tr><tr><td>PB6</td><td>LPUART1 TX</td><td>SB9 and SB18 ON
SB8, SB19, and SB61 OFF</td><td>SB8 and SB19 ON
SB9, SB18, and SB61 OFF</td><td>SB8, SB9, and SB61 OFF</td></tr><tr><td>PB7</td><td>LPUART1 RX</td><td>SB34 and SB66 ON
SB12 and SB68 OFF</td><td>SB12 and SB68 ON
SB34 and SB66 OFF</td><td>SB34 and SB68 OFF</td></tr></table>

Hardware connection required forUSARTbootloader:

The STM32H7x3 embeds aUSART bootloader. To use the USART bootloader (USART1), hardware modifications are required on the Nucleo board. Flying wires must be connected between PD8/PD9 (USART3 available on SB19/SB12) and PB10/PB11 (USART1 available on CN15).

# 7.6.6 USBOTG_FS

The STM32H7 Nucleo-144 board supports USB OTG_FS communication via the USB Micro-AB connector (CN13) and the USB power switch (U18) connected to  $V_{BUS}$ .

Warning: USB Micro-AB connector (CN13) cannot power the Nucleo-144 board. To avoid damaging the STM32H7, it is mandatory to power the Nucleo-144 before connecting a USB cable on CN13. Otherwise, there is a risk of current injection on STM32H7 I/Os.

A green LED (LD8) lights in one of these cases:

Power switch (U12) is ON and the STM32H7 Nucleo-144 board works as a USB Host

-  $V_{BUS}$  is powered by another USB Host when the STM32H7 Nucleo-144 board works as a USB Device.

The red LED (LD7) lights if overcurrent occurs when  $+5\mathrm{V}$  is enabled on  $\mathsf{V}_{\mathsf{BUS}}$  in USB Host mode.

Note: 1.ST recommends powering the Nucleo-144 board with an external power supply when using the USB OTG or Host function.

2.SB76 must be ON when using USB OTG_FS.


Table 12. USB pin configuration


<table><tr><td>Pin name</td><td>Function</td><td>Configuration when using USB connector</td><td>Configuration when using ST morpho connector</td><td>Remark</td></tr><tr><td>PA8</td><td>USB SOF</td><td>-</td><td>-</td><td>Test point TP4</td></tr><tr><td>PA9</td><td>USB V_BUS</td><td>SB23 ON</td><td>SB23 OFF</td><td>-</td></tr><tr><td>PA10</td><td>USB ID</td><td>SB24 ON</td><td>SB24 OFF</td><td>-</td></tr><tr><td>PA11</td><td>USB DM</td><td>SB21 ON, SB16 OFF</td><td>SB16 ON, SB21 OFF</td><td>-</td></tr><tr><td>PA12</td><td>USB DP</td><td>SB22 ON, SB17 OFF</td><td>SB17 ON, SB22 OFF</td><td>-</td></tr><tr><td>PD10</td><td>USB PWR EN</td><td>SB77 ON</td><td>SB77 OFF</td><td>-</td></tr><tr><td>PG7</td><td>USB FS OVCR</td><td>SB76 ON</td><td>SB76 OFF</td><td>-</td></tr></table>

ESD protection part is implemented on the USB port because all USB pins on STM32H7 are dedicated to USB port protection only on the STM32H7 Nucleo-144 board. USB pin ID is not used.

# 7.6.7 Ethernet

The STM32H7 Nucleo-144 board supports 10M/100M Ethernet communication by a PHY (U15) and RJ45 connector (CN14). Ethernet PHY is connected to the STM32H7 series microcontroller via the RMII interface. The PHY RMII_REF_CLK generates the 50 MHz clock for the STM32H7 series microcontroller.

Note: 1.JP6 and SB72 must be ON when using Ethernet.

2. The Ethernet PHY must be set in power-down mode (in this mode, the Ethernet PHY reference clock turns off) to achieve the expected low-power mode current. This is done by configuring the Ethernet PHY basic control register (at address 0x00) bit 11 (power down) to 0b1. SB57 can also be OFF to get the same effect.


Table 13. Ethernet pin configuration


<table><tr><td>Pin name</td><td>Function</td><td>Conflict with ST Zio connector signal</td><td>Configuration when using Ethernet</td><td>Configuration when using ST Zio or ST morpho connector</td></tr><tr><td>PA1</td><td>RMII reference clock</td><td>-</td><td>SB57 ON</td><td>SB57 OFF</td></tr><tr><td>PA2</td><td>RMII MDIO</td><td>-</td><td>SB72 ON</td><td>SB72 OFF</td></tr><tr><td>PC1</td><td>RMII MDC</td><td>-</td><td>SB64 ON</td><td>SB64 OFF</td></tr><tr><td>PA7</td><td>RMII RX data valid</td><td>-</td><td>SB31 ON</td><td>SB31 OFF</td></tr><tr><td>PC4</td><td>RMII RXD0</td><td>-</td><td>SB36 ON</td><td>SB36 OFF</td></tr><tr><td>PC5</td><td>RMII RXD1</td><td>-</td><td>SB29 ON</td><td>SB29 OFF</td></tr><tr><td>PG11</td><td>RMII TX enable</td><td>-</td><td>SB27 ON</td><td>SB27 OFF</td></tr><tr><td>PG13</td><td>RXII TXD0</td><td>-</td><td>SB30 ON</td><td>SB30 OFF</td></tr><tr><td>PB13</td><td>RMII TXD1</td><td>I2S_A_CK</td><td>JP6 ON</td><td>JP6 OFF</td></tr></table>

# 7.7 Solder bridges and jumpers

SBxx can be found on the top layer and SB1xx can be found on the bottom layer of the Nucleo-144 board.


Table 14. Solder bridge and jumper configuration


<table><tr><td>Bridge</td><td>\(State^{(1)}\)</td><td>Description</td></tr><tr><td rowspan="2">SB1 (3V3_PER)</td><td>ON</td><td>Peripheral power 3V3_PER is connected to 3V3.</td></tr><tr><td>OFF</td><td>Peripheral power 3V3_PER is not connected.</td></tr><tr><td rowspan="2">SB2 (3V3)</td><td>ON</td><td>Output of voltage regulator ST1L05CPU33R is connected to 3V3.</td></tr><tr><td>OFF</td><td>Output of voltage regulator ST1L05CPU33R is not connected.</td></tr><tr><td rowspan="2">SB80 (1V8_VDD)</td><td>ON</td><td>Output of voltage regulator ST1L05BPUR is connected to 1V8_VDD.</td></tr><tr><td>OFF</td><td>Output of voltage regulator ST1L05BPUR is not connected.</td></tr><tr><td rowspan="2">SB6</td><td>ON</td><td>Input of voltage regulator ST1L05BPUR is connected to 3V3_VDD.</td></tr><tr><td>OFF</td><td>Input of voltage regulator ST1L05BPUR is not connected.</td></tr><tr><td rowspan="2">SB12, SB19 (ST-LINK-USART)</td><td>ON</td><td>PG9 and PG14 on ST-LINK STM32F723IEK6 are connected to PD8 and PD9 to enable the Virtual COM port. Thus, PD8 and PD9 on the ST morpho connectors cannot be used.</td></tr><tr><td>OFF</td><td>PG9 and PG14 on ST-LINK STM32F723IEK6 are disconnected from PD8 and PD9 on STM32H7.</td></tr><tr><td rowspan="2">JP1 (ST-LINK_RST)</td><td>OFF</td><td>No incidence on ST-LINK STM32F723IEK6 NRST signal.</td></tr><tr><td>ON</td><td>ST-LINK STM32F723IEK6 signal is connected to GND (ST-LINK reset to reduce power consumption).</td></tr><tr><td rowspan="2">SB32(SWO)</td><td>ON</td><td>SWO signal of the STM32H7 (PB3) is connected to the ST-LINK SWO input.(SB26 must be OFF)</td></tr><tr><td>OFF</td><td>SWO signal of STM32H7 is not connected.</td></tr><tr><td rowspan="2">JP3(NRST)</td><td>ON</td><td>Board RESET signal (NRST) is connected to ST-LINK reset control I/O (T_NRST).</td></tr><tr><td>OFF</td><td>Board RESET signal (NRST) is not connected to ST-LINK reset control I/O (T_NRST).</td></tr><tr><td rowspan="3">SB10, SB11, SB20(IOREF)</td><td>OFF, ON,OFF</td><td>IOREF is connected to VDD MCU.</td></tr><tr><td>ON, OFF,OFF</td><td>IOREF is connected to 3V3_PER.</td></tr><tr><td>OFF, OFF,ON</td><td>IOREF is connected to 3V3.</td></tr></table>


Table 14. Solder bridge and jumper configuration (continued)


<table><tr><td>Bridge</td><td>\(State^{(1)}\)</td><td>Description</td></tr><tr><td rowspan="2">SB14 (SDMMC_D0), SB15 (SDMMC_D1)</td><td>ON</td><td>These pins are connected to the ST morpho connector (CN12).</td></tr><tr><td>OFF</td><td>These pins are disconnected from the ST morpho connector (CN12) to avoid stubs of SDMMC data signals on the PCB.</td></tr><tr><td rowspan="4">SB39, SB47 (LD1-LED)</td><td>ON, OFF</td><td>Green user LED (LD1) is connected to PB0.</td></tr><tr><td>OFF, ON</td><td>Green user LED (LD1) is connected to D13 of the ARDUINO® signal (PA5).</td></tr><tr><td>OFF, OFF</td><td>Green user LED (LD1) is not connected.</td></tr><tr><td>ON, ON</td><td>Forbidden</td></tr><tr><td rowspan="2">SB33, SB35 (D11)</td><td>OFF, ON</td><td>D11 (pin 14 of CN7) is connected to STM32H7 PB5 (SPI_A_MOSI/TIM_D_PWM2)</td></tr><tr><td>ON, OFF</td><td>D11 (pin 14 of CN7) is connected to STM32H7 PA7 (SPI_A_MOSI/TIM_E_PWM1)</td></tr><tr><td rowspan="2">SB40, SB41 (X2 crystal)</td><td>OFF, OFF</td><td>PC14 and PC15 are not connected to the ST morpho connector (CN11). X2 is used to generate the 32 kHz clock.</td></tr><tr><td>ON, ON</td><td>PC14 and PC15 are connected to the ST morpho connector (CN11). R38 and R39 must be OFF.</td></tr><tr><td rowspan="3">SB44 (PF1/PH1) SB46 (PF0/PH0) (main clock)</td><td>ON, OFF</td><td>PF0/PH0 is not connected to the ST morpho connector (CN11). PF1/PH1 is connected to the ST morpho connector (MCO is used as the main clock for STM32H7 on PF0/PH0-SB45 ON).</td></tr><tr><td>OFF, OFF</td><td>PF0/PH0 and PF1/PH1 are not connected to the ST morpho connector (CN11). X3, C69, C70, SB3, and SB4 provide a clock. In this case, SB45 must be OFF.</td></tr><tr><td>ON, ON</td><td>PF0/PH0 and PF1/PH1 are connected to the ST morpho connector (CN11). SB3, SB4, and SB45 must be OFF.</td></tr><tr><td rowspan="2">SB45 (STLK_MCO)</td><td>ON</td><td>MCO of ST-LINK (STM32F723IEK6) is connected to PF0/PH0 of STM32H7.</td></tr><tr><td>OFF</td><td>MCO of ST-LINK (STM32F723IEK6) is not connected to PF0/PH0 of STM32H7.</td></tr><tr><td rowspan="2">SB3, SB4 (external 25M crystal)</td><td>OFF, OFF</td><td>PF0/PH0 and PF1/PH1 are not connected to an external 25 MHz crystal X3.</td></tr><tr><td>ON, ON</td><td>PF0/PH0 and PF1/PH1 are connected to an external 25 MHz crystal X3.</td></tr><tr><td rowspan="2">SB52 (VBAT)</td><td>ON</td><td>VBAT pin of STM32H7 is connected to VDD MCU·</td></tr><tr><td>OFF</td><td>VBAT pin of STM32H7 is not connected to VDD MCU·</td></tr><tr><td rowspan="3">SB51, SB58 (B1-USER)</td><td>ON, OFF</td><td>B1 push-button is connected to PC13.</td></tr><tr><td>OFF, ON</td><td>B1 push-button is connected to PA0 (set SB51 OFF if the ST Zio connector is used).</td></tr><tr><td>OFF, OFF</td><td>B1 push-button is not connected.</td></tr></table>


Table 14. Solder bridge and jumper configuration (continued)


<table><tr><td>Bridge</td><td>\(State^{(1)}\)</td><td>Description</td></tr><tr><td rowspan="2">SB75(PA0)</td><td>ON</td><td>PA0 is connected to the ST Zio connector (pin 29 of CN10).</td></tr><tr><td>OFF</td><td>PA0 is not connected to the ST Zio connector (pin 29 of CN10).</td></tr><tr><td rowspan="2">RMII signalsSB57 (PA1), SB64(PC1),SB72 (PA2), SB36(PC4),SB29 (PC5), SB30(PG13), SB27 (PG11),SB31 (PA7), JP6(PB13)</td><td>ON</td><td>These pins are used as RMII signals and connected to Ethernet PHY. SB7 must be OFF.These pins must not be used on the ST morpho or the ST Zio connectors.</td></tr><tr><td>OFF</td><td>These pins can be used as GPIOs on the ST morpho connectors.PB13 can be used as I2S_A_CK on ST Zio (pin 5 of CN7) if not used on the ST morpho.</td></tr><tr><td rowspan="2">SB74 (Ethernet nRST)RMII signal</td><td>ON</td><td>NRST of STM32H7 is connected to Ethernet PHY (U15).</td></tr><tr><td>OFF</td><td>NRST of STM32H7 is not connected to Ethernet PHY (U15).</td></tr><tr><td rowspan="2">SB76 (PG7)</td><td>ON</td><td>USB overcurrent alarm is connected.</td></tr><tr><td>OFF</td><td>USB overcurrent alarm is not connected. PG7 is used as GPIO on the ST morpho connector (CN12).</td></tr><tr><td rowspan="2">SB77 (PD10)</td><td>ON</td><td>PD10 is connected to the USB power switch (U18) to control \(V_{BUS}\).</td></tr><tr><td>OFF</td><td>PD10 is used as GPIO on the ST morpho connector (CN12).</td></tr><tr><td rowspan="2">SB23 (PA9)</td><td>ON</td><td>PA9 is connected to USB \(V_{BUS}\).</td></tr><tr><td>OFF</td><td>PA9 is not connected to USB \(V_{BUS}\).PA9 is used as GPIO on the ST morpho connector (CN12).</td></tr><tr><td rowspan="2">SB24 (PA10)</td><td>ON</td><td>PA10 is connected to the USB ID.</td></tr><tr><td>OFF</td><td>PA10 is not connected to the USB ID.PA10 is used as GPIO on the ST morpho connector (CN12).</td></tr><tr><td rowspan="2">SB21 (PA11), SB22(PA12)</td><td>ON</td><td>These pins are used as D- and D+ on the USB connector (CN13). SB16 and SB17 must be OFF.</td></tr><tr><td>OFF</td><td>These pins are used as GPIOs on the ST morpho connectors. SB16 and SB17 must be ON.</td></tr><tr><td rowspan="2">SB13</td><td>ON</td><td>VDD33_USB_1 is connected to 3V3_VDD.</td></tr><tr><td>OFF</td><td>VDD33_USB_1 is not supplied.</td></tr><tr><td rowspan="2">SB25</td><td>ON</td><td>VDD_MMC_1 is connected to VDD MCU.</td></tr><tr><td>OFF</td><td>VDD_MMC_1 is not supplied.</td></tr><tr><td rowspan="2">SB59 (PG6)</td><td>ON</td><td>PG6 is connected to QSPI_CS. SB61 must be OFF.</td></tr><tr><td>OFF</td><td>PG6 is used as GPIO on the ST morpho connector (CN12).</td></tr><tr><td rowspan="2">SB63 (PB2)</td><td>ON</td><td>PB2 is connected to QSPI_CLK. SB69 must be OFF.</td></tr><tr><td>OFF</td><td>PB2 is not connected to QSPI_CLK and can be used as COMP1_INP (SB69 ON) or used as GPIO on the ST morpho connector (CN12). SB69 must be OFF.</td></tr></table>


Table 14. Solder bridge and jumper configuration (continued)


<table><tr><td>Bridge</td><td>\(State^{(1)}\)</td><td>Description</td></tr><tr><td rowspan="2">SB71, SB73 (PE6)</td><td>ON, OFF</td><td>PE6 is connected to SAI_A_SD (D59 of CN9)</td></tr><tr><td>OFF, ON</td><td>PE6 is connected to TIMER_A_BKIN2 (D38 of CN10)</td></tr><tr><td rowspan="2">SB67 (PE2)</td><td>ON</td><td>PE2 is connected to SAI_A_MCLK (D56 of CN9). QSPI_BK1_IO2 cannot be used (D31 of CN10).</td></tr><tr><td>OFF</td><td>PE2 is used as QSPI_BK1_IO2 (D31 of CN10).</td></tr><tr><td rowspan="2">SB53 (PC2) and SB60 (PF10)</td><td>ON</td><td>ADC_IN are connected to A4 and A5 (pins 9 and 11) on the ST Zio connector (CN9). Thus, SB55 and SB62 must be OFF</td></tr><tr><td>OFF</td><td>ADC_IN are connected to A4 and A5 (pins 9 and 11) on the ST Zio connector (CN9). Thus, SB55 and SB62 can be ON (\(I^2C\))</td></tr><tr><td>SB65 (PF11)</td><td>OFF</td><td>On NUCLEO-H723ZG, NUCLEO-H743ZI2, and NUCLEO-H753ZI, PF11 is used only as GPIO on the ST morpho connector (CN12). It must not be used as ADC_IN.</td></tr><tr><td rowspan="2">\(I^2C\)SB55 (PB9) and SB62 (PB8)</td><td>ON</td><td>PB9 and PB8 (\(I^2C\)) are connected to A4 and A5 (pins 9 and 11) on the ST Zio connector (CN9). Thus, SB60 and SB53 must be OFF</td></tr><tr><td>OFF</td><td>PB9 and PB8 (\(I^2C\)) are not connected to A4 and A5 (pins 9 and 11) on the ST Zio connector (CN9).</td></tr><tr><td rowspan="2">SB28 and SB70 (PE9)</td><td>ON, OFF</td><td>PE9 is used as TIMER_A_PWM1 (pin 4) on the ST Zio connector (CN10).</td></tr><tr><td>OFF, ON</td><td>PE9 is used as COMP2_INP (pin 15) on the ST Zio connector (CN9).</td></tr><tr><td>SB37 (PF12) and SB38 (PF4)</td><td>OFF, ON</td><td>ADC_IN is connected to A6 (pin 7) on the ST Zio connector (CN10). PF12 must not be used as ADC_IN. SB37 must be OFF.</td></tr><tr><td>SB48 (PF5) and SB49 (PF13)</td><td>ON, OFF</td><td>ADC_IN is connected to A7 (pin 9) on the ST Zio connector (CN10). PF13 must not be used as ADC_IN. SB49 must be OFF.</td></tr><tr><td>SB50 (PF14) and SB54 (PF6)</td><td>OFF, ON</td><td>ADC_IN is connected to A8 (pin 11) on the ST Zio connector (CN10). PF14 must not be used as ADC_IN. SB50 must be OFF.</td></tr><tr><td rowspan="2">SB5</td><td>OFF</td><td>NUCLEO-H723ZG, NUCLEO-H743ZI2, and NUCLEO-H753ZI support 1V8 and 3V3 for VDD MCU. Thus, the level shifter (U10) is needed and SB5 must be OFF.</td></tr><tr><td>ON</td><td>If the MCU is supplied with 3V3, U10 can be by-passed and SB5 can be ON.</td></tr><tr><td rowspan="2">SB81</td><td>ON</td><td>USART_RX connected to ST morpho (CN11)</td></tr><tr><td>OFF</td><td>USART_RX not connected to ST morpho (CN11)</td></tr><tr><td rowspan="2">SB82</td><td>ON</td><td>USART_TX connected to ST morpho (CN12)</td></tr><tr><td>OFF</td><td>USART_TX not connected to ST morpho (CN12)</td></tr></table>


1. The default SBx state is shown in bold.


All the other solder bridges present on the STM32H7 Nucleo-144 board are used to configure several I/Os and power supply pins for compatibility of features and pinout with the supported target STM32H7.

The STM32H7 Nucleo-144 board is delivered with the solder bridges configured, according to the supported target STM32H7.

# 8 Board connectors

Several connectors are implemented on the STM32H7 Nucleo-144 board.

# 8.1 STLINK-V3E USB Micro-B connector (CN1)

The USB Micro-B connector (CN1) is used to connect embedded STLINK-V3E to the PC for programming and debugging purposes.

The related pinout for the USB STLINK-V3E connector is listed in Table 15.


Table 15. USB Micro-B connector (CN1) pinout


<table><tr><td>Connector</td><td>Pin number</td><td>Pin name</td><td>Signal name</td><td>ST-LINK MCU pin</td><td>Function</td></tr><tr><td rowspan="5">CN1</td><td>1</td><td>VBUS</td><td>5V_USB_CHGR</td><td>-</td><td>5 V power</td></tr><tr><td>2</td><td>DM</td><td>USB_DEV_HS_CN_N</td><td>PB14</td><td>USB differential pair N</td></tr><tr><td>3</td><td>DP</td><td>USB_DEV_HS_CN_P</td><td>PB15</td><td>USB differential pair P</td></tr><tr><td>4</td><td>ID</td><td>-</td><td>-</td><td>-</td></tr><tr><td>5</td><td>GND</td><td>-</td><td>-</td><td>GND</td></tr></table>

# 8.2 USB OTG_FS connector (CN13)

A USB OTG full-speed communication link is available at the USB Micro-AB receptacle connector (CN13). Micro-AB receptacle enables USB Host and USB Device features.

The related pinout for the USB OTG_FS connector is listed in Table 16.


Table 16. USB OTG_FS Micro-AB connector (CN13) pinout


<table><tr><td>Connector</td><td>Pin number</td><td>Pin name</td><td>Signal name</td><td>MCU pin</td><td>Function</td></tr><tr><td rowspan="5">CN13</td><td>1</td><td>VBUS</td><td>USB_FS_VBUS</td><td>PA9</td><td>5 V power</td></tr><tr><td>2</td><td>DM</td><td>USB_FS_N</td><td>PA11</td><td>USB differential pair M</td></tr><tr><td>3</td><td>DP</td><td>USB_FS_P</td><td>PA12</td><td>USB differential pair P</td></tr><tr><td>4</td><td>ID</td><td>USB_FS_ID</td><td>PA10</td><td>-</td></tr><tr><td>5</td><td>GND</td><td>-</td><td>-</td><td>GND</td></tr></table>

# 8.3 Ethernet RJ45 connector (CN14)

The STM32H7 Nucleo-144 board supports 10Mbps/100Mbps Ethernet communication with the PHY (U15) and integrated RJ45 connector (CN14). The Ethernet PHY is connected to the MCU via the RMII interface.

The X4 oscillator generates the 25 MHz clock for the PHY. The 50 MHz clock for the MCU (derived from the 25 MHz crystal oscillator) is provided by the RMII_REF_CLK of the PHY.


Figure 15. Ethernet RJ45 connector (CN14) front view


![](images/88e6b2cbbc03a9ed807ed93e935b6a2711060f6fdf61974e1d4bc064e05c3ce1.jpg)



12. 1


1. Green LED: Ethernet traffic

2. Amber LED: Ethernet connection

The related pinout for the Ethernet connector is listed in Table 17.


Table 17. Ethernet RJ45 connector (CN14) pinout


<table><tr><td>Connector</td><td>Pin number</td><td>Description</td><td>MCU pin</td><td>Pin number</td><td>Description</td><td>MCU pin</td></tr><tr><td rowspan="6">CN14</td><td>1</td><td>TX+</td><td>-</td><td>7</td><td>NC</td><td>-</td></tr><tr><td>2</td><td>TX-</td><td>-</td><td>8</td><td>NC</td><td>-</td></tr><tr><td>3</td><td>RX+</td><td>-</td><td>9</td><td>K, yellow LED</td><td>-</td></tr><tr><td>4</td><td>NC</td><td>-</td><td>10</td><td>A, yellow LED</td><td>-</td></tr><tr><td>5</td><td>NC</td><td>-</td><td>11</td><td>K, green LED</td><td>-</td></tr><tr><td>6</td><td>RX-</td><td>-</td><td>12</td><td>A, green LED</td><td>-</td></tr></table>

# 9 Extension connectors

# 9.1 ST Zio connectors

For all STM32H7 Nucleo-144 boards, Figure 16 shows the signals connected by default to the ST Zio connectors (CN7, CN8, CN9, and CN10), including the support of ARDUINO® Uno V3.


Figure 16. NUCLEOH7 Nucleo-144 board


![](images/c8e4a846caaa98034176fa527b0c1351c8d076b2bee212f7d07b111cf926b49b.jpg)


CN7, CN8, CN9, and CN10 are female connectors on the top side and male connectors on the bottom side. They include support for ARDUINO® Uno V3. Most shields designed for ARDUINO® Uno V3 can fit the STM32H7 Nucleo-144 board.

To cope with ARDUINO® Uno V3, apply the following modifications:

- SB55 and SB62 must be ON

- SB53/60/65 must be OFF to connect I²C on A4 (pin 9) and A5 (pin 11 of CN9).

Caution:1 The I/Os of the STM32H7 series microcontroller are  $3.3\mathrm{V}$  compatible instead of  $5\mathrm{V}$  for ARDUINO® Uno V3.

Caution:2 R37 must be OFF before implementing the ARDUINO® shield with  $V_{\mathrm{REF+}}$  power provided on CN7 pin 6. Refer to Table 14: Solder bridge and jumper configuration for details on R37.

# NUCLEO-H723ZG, NUCLEO-H743ZI2, and NUCLEO-H753ZI pin assignments


Table 18. ZIO connector (CN7) pinout(1)


<table><tr><td>Pin</td><td>Pin name</td><td>Signal name</td><td>STM32H7 pin</td><td>MCU function</td><td>Pin</td><td>Pin name</td><td>Signal name</td><td>STM32H7 pin</td><td>MCU function</td></tr><tr><td>1</td><td>D16</td><td>I2S_A_MCK</td><td>PC6</td><td>I2S_2</td><td>2</td><td>D15</td><td>I2C_A_SCL</td><td>PB8</td><td>I2C_1_SCL</td></tr><tr><td>3</td><td>D17</td><td>I2S_A_SD</td><td>PB15</td><td>I2S_2</td><td>4</td><td>D14</td><td>I2C_A_SDA</td><td>PB9</td><td>I2C_1_SDA</td></tr><tr><td>5</td><td>D18</td><td>I2S_A_CK</td><td>PB13(2)</td><td>I2S_2</td><td>6</td><td>VREFP</td><td>VREFP</td><td>-</td><td>VDDA/VREFP</td></tr><tr><td>7</td><td>D19</td><td>I2S_A_WS</td><td>PB12</td><td>I2S_2</td><td>8</td><td>GND</td><td>GND</td><td>-</td><td>-</td></tr><tr><td>9</td><td>D20</td><td>I2S_B_WS</td><td>PA15</td><td>I2S_3</td><td>10</td><td>D13</td><td>SPI_A_SCK</td><td>PA5</td><td>SPI1_SCK</td></tr><tr><td>11</td><td>D21</td><td>I2S_B_MCK</td><td>PC7</td><td>I2S_3</td><td>12</td><td>D12</td><td>SPI_A_MISO</td><td>PA6</td><td>SPI1_MISO</td></tr><tr><td>13</td><td>D22</td><td>I2S_B_SD/SPI_B_MOSI</td><td>PB5</td><td>I2S_3/SPI3</td><td>14</td><td>D11</td><td>SPI_A_MOSI/TIM_E_PWM1</td><td>PB5(3)</td><td>SPI1_MOSI/TIM3_CH2</td></tr><tr><td>15</td><td>D23</td><td>I2S_B_CK/SPI_B_SCK</td><td>PB3</td><td>I2S_3/SPI3</td><td>16</td><td>D10</td><td>SPI_A_CS/TIM_B_PWM3</td><td>PD14</td><td>SPI1_CS/TIM4_CH3</td></tr><tr><td>17</td><td>D24</td><td>SPI_B_NSS</td><td>PA4</td><td>SPI3</td><td>18</td><td>D9</td><td>TIM_B_PWM2</td><td>PD15</td><td>TIM4_CH4</td></tr><tr><td>19</td><td>D25</td><td>SPI_B_MISO</td><td>PB4</td><td>SPI3</td><td>20</td><td>D8</td><td>I/O</td><td>PF3</td><td>-</td></tr></table>


1. For more details, refer to Table 14: Solder bridge and jumper configuration.



2. PB13 is used as I2S_A_CK and connected to CN7 pin 5. If JP6 is ON, it is also connected to Ethernet PHY as RMII_TXD1. In this case, only one function of the Ethernet or I2S_A must be used.



3. PA7 is used as D11 and connected to CN7 pin 14. If SB31 is ON, it is also connected to Ethernet PHY as RMII_CRS_DV. In this case, only one function of the Ethernet or D11 must be used.



Table 19. ZIO connector (CN8) pinout


<table><tr><td>Pin</td><td>Pin name</td><td>Signal name</td><td>STM32H7 pin</td><td>MCU function</td><td>Pin</td><td>Pin name</td><td>Signal name</td><td>STM32H7 pin</td><td>MCU function</td></tr><tr><td>1</td><td>NC</td><td>NC</td><td>-</td><td>-</td><td>2</td><td>D43</td><td>SDMMC_D0</td><td>PC8</td><td>SDMMC</td></tr><tr><td>3</td><td>IOREF</td><td>IOREF</td><td>-</td><td>3.3 V reference</td><td>4</td><td>D44</td><td>SDMMC_D1 I2S_A_CKIN</td><td>PC9</td><td>SDMMC I2S_CKIN</td></tr><tr><td>5</td><td>NRST</td><td>NRST</td><td>NRST</td><td>RESET</td><td>6</td><td>D45</td><td>SDMMC_D2</td><td>PC10</td><td>SDMMC</td></tr></table>


Table 19. ZIO connector (CN8) pinout (continued)


<table><tr><td>Pin</td><td>Pin name</td><td>Signal name</td><td>STM32H7 pin</td><td>MCU function</td><td>Pin</td><td>Pin name</td><td>Signal name</td><td>STM32H7 pin</td><td>MCU function</td></tr><tr><td>7</td><td>3V3</td><td>3V3</td><td>-</td><td>3.3 V input/output</td><td>8</td><td>D46</td><td>SDMMC_D3</td><td>PC11</td><td>SDMMC</td></tr><tr><td>9</td><td>5V</td><td>5V</td><td>-</td><td>5 V output</td><td>10</td><td>D47</td><td>SDMMC_CK</td><td>PC12</td><td>SDMMC</td></tr><tr><td>11</td><td>GND</td><td>GND</td><td>-</td><td>ground</td><td>12</td><td>D48</td><td>SDMMC_CMD</td><td>PD2</td><td>SDMMC</td></tr><tr><td>13</td><td>GND</td><td>GND</td><td>-</td><td>ground</td><td>14</td><td>D49</td><td>I/O</td><td>PG2</td><td>-</td></tr><tr><td>15</td><td>VIN</td><td>VIN</td><td>-</td><td>Power input</td><td>16</td><td>D50</td><td>I/O</td><td>PG3</td><td>-</td></tr></table>


Table 20. ZIO connector (CN9) pinout


<table><tr><td>Pin</td><td>Pin name</td><td>Signal name</td><td>STM32H7 pin</td><td>MCU function</td><td>Pin</td><td>Pin name</td><td>Signal name</td><td>STM32H7 pin</td><td>MCU function</td></tr><tr><td>1</td><td>A0</td><td>ADC</td><td>PA3</td><td>ADC12_INP15</td><td>2</td><td>D51</td><td>USART_B_SCL</td><td>PD7</td><td>USART_2</td></tr><tr><td>3</td><td>A1</td><td>ADC</td><td>PC0</td><td>ADC123_INP10</td><td>4</td><td>D52</td><td>USART_B_RX</td><td>PD6</td><td>USART_2</td></tr><tr><td>5</td><td>A2</td><td>ADC</td><td>PC3</td><td>ADC12_INP13</td><td>6</td><td>D53</td><td>USART_B_TX</td><td>PD5</td><td>USART_2</td></tr><tr><td>7</td><td>A3</td><td>ADC</td><td>PB1</td><td>ADC12_INP5</td><td>8</td><td>D54</td><td>USART_B_RTS</td><td>PD4</td><td>USART_2</td></tr><tr><td>9</td><td>A4</td><td>ADC</td><td>PC2/ PB9</td><td>ADC123_INP12/ I2C1_SDA</td><td>10</td><td>D55</td><td>USART_B_CTS</td><td>PD3</td><td>USART_2</td></tr><tr><td>11</td><td>A5</td><td>ADC</td><td>PF10/ PB8</td><td>ADC3_INP6/ I2C1_SCL</td><td>12</td><td>GND</td><td>GND</td><td>-</td><td>-</td></tr><tr><td>13</td><td>D72</td><td>COMP1_INP</td><td>PB2</td><td>COMP1_INP</td><td>14</td><td>D56</td><td>SAI_A_MCLK</td><td>PE2(1)</td><td>SAI_1_A</td></tr><tr><td>15</td><td>D71</td><td>COMP2_INP</td><td>PE9</td><td>COMP2_INP</td><td>16</td><td>D57</td><td>SAI_A_FS</td><td>PE4</td><td>SAI_1_A</td></tr><tr><td>17</td><td>D70</td><td>I2C_B_SMBA</td><td>PF2</td><td>I2C2</td><td>18</td><td>D58</td><td>SAI_A_SCK</td><td>PE5</td><td>SAI_1_A</td></tr><tr><td>19</td><td>D69</td><td>I2C_B_SCL</td><td>PF1</td><td>I2C2</td><td>20</td><td>D59</td><td>SAI_A_SD</td><td>PE6</td><td>SAI_1_A</td></tr><tr><td>21</td><td>D68</td><td>I2C_B_SDA</td><td>PF0</td><td>I2C2</td><td>22</td><td>D60</td><td>SAI_B_SD</td><td>PE3</td><td>SAI_1_B</td></tr><tr><td>23</td><td>GND</td><td>GND</td><td>-</td><td>-</td><td>24</td><td>D61</td><td>SAI_B_SCK</td><td>PF8</td><td>SAI_1_B</td></tr><tr><td>25</td><td>D67</td><td>CAN_RX</td><td>PD0</td><td>CAN_1</td><td>26</td><td>D62</td><td>SAI_B_MCLK</td><td>PF7</td><td>SAI_1_B</td></tr></table>


Table 20. ZIO connector (CN9) pinout (continued)


<table><tr><td>Pin</td><td>Pin name</td><td>Signal name</td><td>STM32H7 pin</td><td>MCU function</td><td>Pin</td><td>Pin name</td><td>Signal name</td><td>STM32H7 pin</td><td>MCU function</td></tr><tr><td>27</td><td>D66</td><td>CAN_TX</td><td>PD1</td><td>CAN_1</td><td>28</td><td>D63</td><td>SAI_B_FS</td><td>PF9</td><td>SAI_1_B</td></tr><tr><td>29</td><td>D65</td><td>I/O</td><td>PG0</td><td>-</td><td>30</td><td>D64</td><td>I/O</td><td>PG1</td><td>-</td></tr></table>


1. PE2 is connected to both CN9 pin 14 (SAI_A_MCLK) and CN10 pin 25 (QSPI_BK1_IO2). Only one function must be used at one time.



Table 21. ZIO connector (CN10) pinout


<table><tr><td>Pin</td><td>Pin name</td><td>Signal name</td><td>STM32H7 pin</td><td>MCU function</td><td>Pin</td><td>Pin name</td><td>Signal name</td><td>STM32H7 pin</td><td>MCU function</td></tr><tr><td>1</td><td>AVDD</td><td>VDDA</td><td>-</td><td>Analog VDD</td><td>2</td><td>D7</td><td>I/O</td><td>PG12</td><td>I/O</td></tr><tr><td>3</td><td>AGND</td><td>AGND</td><td>-</td><td>Analog GND</td><td>4</td><td>D6</td><td>TIMER_A_PWM1</td><td>PE9</td><td>TIM1_CH1</td></tr><tr><td>5</td><td>GND</td><td>GND</td><td>-</td><td>GND</td><td>6</td><td>D5</td><td>TIMER_A_PWM2</td><td>PE11</td><td>TIM1_CH2</td></tr><tr><td>7</td><td>A6</td><td>ADC_A_IN</td><td>PF4</td><td>ADC3_INP9</td><td>8</td><td>D4</td><td>I/O</td><td>PE14</td><td>I/O</td></tr><tr><td>9</td><td>A7</td><td>ADC_B_IN</td><td>PF5</td><td>ADC3_INP4</td><td>10</td><td>D3</td><td>TIMER_A_PWM3</td><td>PE13</td><td>TIM1_CH3</td></tr><tr><td>11</td><td>A8</td><td>ADC_C_IN</td><td>PF6</td><td>ADC3_INP8</td><td>12</td><td>D2</td><td>I/O</td><td>PG14</td><td>I/O</td></tr><tr><td>13</td><td>D26</td><td>QSPI_CS</td><td>PG6</td><td>QSPI1_NCS</td><td>14</td><td>D1</td><td>USART_A_TX</td><td>PB6</td><td>LPUART1</td></tr><tr><td>15</td><td>D27</td><td>QSPI_CLK</td><td>PB2</td><td>QSPI1_CLK</td><td>16</td><td>D0</td><td>USART_A_RX</td><td>PB7</td><td>LPUART1</td></tr><tr><td>17</td><td>GND</td><td>GND</td><td>-</td><td>GND</td><td>18</td><td>D42</td><td>TIMER_A_PWM1N</td><td>PE8</td><td>TIM1_CH1N</td></tr><tr><td>19</td><td>D28</td><td>QSPI_BK1_IO3</td><td>PD13</td><td>QSPI1_IO</td><td>20</td><td>D41</td><td>TIMER_A_ETR</td><td>PE7</td><td>TIM1_ETR</td></tr><tr><td>21</td><td>D29</td><td>QSPI_BK1_IO1</td><td>PD12</td><td>QSPI1_IO</td><td>22</td><td>GND</td><td>GND</td><td>-</td><td>GND</td></tr><tr><td>23</td><td>D30</td><td>QSPI_BK1_IO0</td><td>PD11</td><td>QSPI1_IO</td><td>24</td><td>D40</td><td>TIMER_A_PWM2N</td><td>PE10</td><td>TIM1_CH2N</td></tr><tr><td>25</td><td>D31</td><td>QSPI_BK1_IO2</td><td>PE2(1)</td><td>QSPI1_IO</td><td>26</td><td>D39</td><td>TIMER_A_PWM3N</td><td>PE12</td><td>TIM1_CH3N</td></tr><tr><td>27</td><td>GND</td><td>-</td><td>-</td><td>-</td><td>28</td><td>D38</td><td>TIMER_A_BKIN2</td><td>PE6</td><td>TIM1_BKIN2</td></tr><tr><td>29</td><td>D32</td><td>TIM_C_PWM1</td><td>PA0</td><td>TIM2_CH1</td><td>30</td><td>D37</td><td>TIMER_A_BKIN1</td><td>PE15</td><td>TIM1_BKIN1</td></tr><tr><td>31</td><td>D33</td><td>TIM_D_PWM1</td><td>PB0</td><td>TIM3_CH3</td><td>32</td><td>D36</td><td>TIMER_C_PWM2</td><td>PB10</td><td>TIM2_CH3</td></tr><tr><td>33</td><td>D34</td><td>TIM_B_ETR</td><td>PE0</td><td>TIM4_ETR</td><td>34</td><td>D35</td><td>TIMER_C_PWM3</td><td>PB11</td><td>TIM2_CH4</td></tr></table>

# 9.2 ST morpho connector

The ST morpho connector consists of two male pin header footprints (CN11 and CN12, both soldered by default). They are used to connect the STM32H7 Nucleo-144 board to an extension board or a prototype/wrapping board placed on the top of the STM32H7 Nucleo-144 board. All signals and power pins of the STM32H7 are available on the ST morpho connector. An oscilloscope, logical analyzer, or voltmeter can also probe this connector.

Table 22 shows the pin assignments of each STM32H7 on the ST morpho connector.


Table 22. Pin assignment of the ST morpho connector


<table><tr><td colspan="2">CN11 odd pins</td><td colspan="2">CN11 even pins</td><td colspan="2">CN12 odd pins</td><td colspan="2">CN12 even pins</td></tr><tr><td>Pin nbr</td><td>Pin name</td><td>Pin nbr</td><td>Pin name</td><td>Pin nbr</td><td>Pin name</td><td>Pin nbr</td><td>Pin name</td></tr><tr><td>1</td><td>PC10</td><td>2</td><td>PC11</td><td>1</td><td>PC9</td><td>2</td><td>PC8</td></tr><tr><td>3</td><td>PC12</td><td>4</td><td>PD2</td><td>3</td><td>PB8</td><td>4</td><td>PC6</td></tr><tr><td>5</td><td>3V3_VDD</td><td>6</td><td>5V_EXT</td><td>5</td><td>PB9</td><td>6</td><td>PC5</td></tr><tr><td>7</td><td>BOOT0(1)</td><td>8</td><td>GND</td><td>7</td><td>VREFP</td><td>8</td><td>5V_USB_STLK(2)</td></tr><tr><td>9</td><td>PF6</td><td>10</td><td>NC</td><td>9</td><td>GND</td><td>10</td><td>PD8</td></tr><tr><td>11</td><td>PF7</td><td>12</td><td>IOREF</td><td>11</td><td>PA5</td><td>12</td><td>PA12</td></tr><tr><td>13</td><td>PA13(3)</td><td>14</td><td>NRST</td><td>13</td><td>PA6</td><td>14</td><td>PA11</td></tr><tr><td>15</td><td>PA14(3)</td><td>16</td><td>3V3</td><td>15</td><td>PA7</td><td>16</td><td>PB12</td></tr><tr><td>17</td><td>PA15</td><td>18</td><td>5V</td><td>17</td><td>PB6</td><td>18</td><td>PB11</td></tr><tr><td>19</td><td>GND</td><td>20</td><td>GND</td><td>19</td><td>PC7</td><td>20</td><td>GND</td></tr><tr><td>21</td><td>PB7</td><td>22</td><td>GND</td><td>21</td><td>PA9</td><td>22</td><td>PB2</td></tr><tr><td>23</td><td>PC13</td><td>24</td><td>VIN</td><td>23</td><td>PA8</td><td>24</td><td>PB1</td></tr><tr><td>25</td><td>PC14</td><td>26</td><td>NC</td><td>25</td><td>PB10</td><td>26</td><td>PB15</td></tr><tr><td>27</td><td>PC15</td><td>28</td><td>PA0</td><td>27</td><td>PB4</td><td>28</td><td>PB14</td></tr><tr><td>29</td><td>PH0</td><td>30</td><td>PA1</td><td>29</td><td>PB5</td><td>30</td><td>PB13</td></tr><tr><td>31</td><td>PH1</td><td>32</td><td>PA4</td><td>31</td><td>PB3</td><td>32</td><td>AGND</td></tr><tr><td>33</td><td>VBAT</td><td>34</td><td>PB0</td><td>33</td><td>PA10</td><td>34</td><td>PC4</td></tr><tr><td>35</td><td>PC2</td><td>36</td><td>PC1</td><td>35</td><td>PA2</td><td>36</td><td>PF5</td></tr><tr><td>37</td><td>PC3</td><td>38</td><td>PC0</td><td>37</td><td>PA3</td><td>38</td><td>PF4</td></tr><tr><td>39</td><td>PD4</td><td>40</td><td>PD3</td><td>39</td><td>GND</td><td>40</td><td>PE8</td></tr><tr><td>41</td><td>PD5</td><td>42</td><td>PG2</td><td>41</td><td>PD13</td><td>42</td><td>PF10</td></tr><tr><td>43</td><td>PD6</td><td>44</td><td>PG3</td><td>43</td><td>PD12</td><td>44</td><td>PE7</td></tr><tr><td>45</td><td>PD7</td><td>46</td><td>PE2</td><td>45</td><td>PD11</td><td>46</td><td>PD14</td></tr><tr><td>47</td><td>PE3</td><td>48</td><td>PE4</td><td>47</td><td>PE10</td><td>48</td><td>PD15</td></tr><tr><td>49</td><td>GND</td><td>50</td><td>PE5</td><td>49</td><td>PE12</td><td>50</td><td>PF14</td></tr><tr><td>51</td><td>PF1</td><td>52</td><td>PF2</td><td>51</td><td>PE14</td><td>52</td><td>PE9</td></tr><tr><td>53</td><td>PF0</td><td>54</td><td>PF8</td><td>53</td><td>PE15</td><td>54</td><td>GND</td></tr></table>


Table 22. Pin assignment of the ST morpho connector (continued)


<table><tr><td colspan="2">CN11 odd pins</td><td colspan="2">CN11 even pins</td><td colspan="2">CN12 odd pins</td><td colspan="2">CN12 even pins</td></tr><tr><td>Pin nbr</td><td>Pin name</td><td>Pin nbr</td><td>Pin name</td><td>Pin nbr</td><td>Pin name</td><td>Pin nbr</td><td>Pin name</td></tr><tr><td>55</td><td>PD1</td><td>56</td><td>PF9</td><td>55</td><td>PE13</td><td>56</td><td>PE11</td></tr><tr><td>57</td><td>PD0</td><td>58</td><td>PG1</td><td>57</td><td>PF13</td><td>58</td><td>PF3</td></tr><tr><td>59</td><td>PG0</td><td>60</td><td>GND</td><td>59</td><td>PF12</td><td>60</td><td>PF15</td></tr><tr><td>61</td><td>PE1</td><td>62</td><td>PE6</td><td>61</td><td>PG14</td><td>62</td><td>PF11</td></tr><tr><td>63</td><td>PG9</td><td>64</td><td>PG15</td><td>63</td><td>GND</td><td>64</td><td>PE0</td></tr><tr><td>65</td><td>PG12</td><td>66</td><td>PG10</td><td>65</td><td>PD10</td><td>66</td><td>PG8</td></tr><tr><td>67</td><td>NC</td><td>68</td><td>PG13</td><td>67</td><td>PG7</td><td>68</td><td>PG5</td></tr><tr><td>69</td><td>PD9</td><td>70</td><td>PG11</td><td>69</td><td>PG4</td><td>70</td><td>PG6</td></tr></table>


1. The default state of BOOT0 is 0. It can be set to 1 when a jumper is plugged into CN11 [5-7].



2. 5V_USB_STLK is the 5 V power coming from the ST-LINKV3 USB connector that rises before and it rises before the +5 V rising on the board.



3. PA13 and PA14 are shared with SWD signals connected to STLINK-V3E. ST does not recommend using them as I/O pins.


# 10 Nucleo-144 boards (MB1364) information

# 10.1 Product marking

The product and each board composing the product are identified with one or several stickers. The stickers, located on the top or bottom side of each PCB, provide product information:

- Main board featuring the target device: product order code, product identification, serial number, and board reference with revision.

Single-sticker example:

Product order code Product identification syywwxxxxx MBxxxx-Variant-yzz

![](images/13b61e84c45492ccbc4127c64af8c58484911966f26f0c9c76595dbad0ee731f.jpg)


Dual-sticker example:

Product order code Product identification

and

MBxxxx-Variant-yzz syywwwwxxx

![](images/0313246b5dfb4964680fbfbb7f6d35ce33b6c02cc9e4e6aedc2407f4b9e7d49d.jpg)


- Other boards if any: board reference with revision and serial number.

Examples:

syywwwxxxxxxxx MBxxxx-Variant-yzz

![](images/ab1f76d576181982f5b46b0d63411b871c082066041b959b198e7e78d3ed355b.jpg)


or

MBxxxx-Variant-yzz syywwwwxxx

![](images/b15c3464895e7654a5b33595228c8b9a69e480029198c3c2434d60204ed374a6.jpg)


or

![](images/a3e6f369f48d897d70e5f697298f6757d9c401ae28c277b3b276f316d31d259f.jpg)


or

![](images/72530a6cc2c78fd1579adc79e632dfe285e55a14980b42663ef95e3d99b9428a.jpg)


On the main board sticker, the first line provides the product order code, and the second line the product identification.

On all board stickers, the line formatted as "MBxxxx-Variant-yzz" shows the board reference "MBxxxx", the mounting variant "Variant" when several exist (optional), the PCB revision "y", and the assembly revision "zz", for example B01. The other line shows the board serial number used for traceability.

Products and parts labeled as "ES" or "E" are not yet qualified or feature devices that are not yet qualified. STMicroelectronics disclaims any responsibility for consequences arising from their use. Under no circumstances will STMicroelectronics be liable for the customer's use of these engineering samples. Before deciding to use these engineering samples for qualification activities, contact STMicroelectronics' quality department.

"ES" or "E" marking examples of location:

- On the targeted STM32 that is soldered on the board (for an illustration of STM32 marking, refer to the STM32 datasheet Package information paragraph at the www.st.com website).

- Next to the evaluation tool ordering part number that is stuck or silk-screen printed on the board.

Some boards feature a specific STM32 device version, which allows the operation of any bundled commercial stack/library available. This STM32 device shows a "U" marking option at the end of the standard part number and is not available for sales.

To use the same commercial stack in their applications, the developers might need to purchase a part number specific to this stack/library. The price of those part numbers includes the stack/library royalties.

# 10.2 Nucleo-144 boards (MB1364) product history


Table 23. Product history


<table><tr><td>Order code</td><td>Product identification</td><td>Product details</td><td>Product change description</td><td>Product limitations</td></tr><tr><td rowspan="6">NUCLEO-H743Z12</td><td rowspan="3">NUH743ZI$AT1</td><td>MCU:- STM32H743ZIT6 revision &#x27;V&#x27;</td><td rowspan="3">Initial revision</td><td rowspan="3">The IDD measurement of the STM32H7x3 MCU cannot be performed in Standby mode because of ETH PHY (U15) and the pull-up resistors (R40, R43, R46, R47, and R48) supplied by VDD MCU.</td></tr><tr><td>MCU errata sheet:- STM32H742xI/G and STM32H743xI/G device limitations (ES0392)</td></tr><tr><td>Board:- MB1364-H743ZI-C01 (main board)</td></tr><tr><td rowspan="3">NUH743ZI$AT2</td><td>MCU:- STM32H743ZIT6 revision &#x27;V&#x27;</td><td rowspan="3">The IDD measurement of the STM32H7x3 MCU can be measured in Standby mode. The ETH PHY is powered by VDD and is separated from VDD MCU. The silkscreen is corrected on the morpho connectors.</td><td rowspan="3">No limitation</td></tr><tr><td>MCU errata sheet:- STM32H742xI/G and STM32H743xI/G device limitations (ES0392)</td></tr><tr><td>Board:- MB1364-H743ZI-E01 (main board)</td></tr><tr><td rowspan="6">NUCLEO-H753Z1</td><td rowspan="3">NUH753ZI$AT1</td><td>MCU:- STM32H753ZIT6 revision &#x27;V&#x27;</td><td rowspan="3">Initial revision</td><td rowspan="3">The IDD measurement of the STM32H7x3 MCU cannot be performed in Standby mode because of ETH PHY (U15) and the pull-up resistors (R40, R43, R46, R47, and R48) supplied by VDD MCU.</td></tr><tr><td>MCU errata sheet:- STM32H750xB and STM32H753xI device limitations (ES0396)</td></tr><tr><td>Board:- MB1364-H753ZI-C01 (main board)</td></tr><tr><td rowspan="3">NUH753ZI$AT2</td><td>MCU:- STM32H753ZIT6 revision &#x27;V&#x27;</td><td rowspan="3">The IDD measurement of the STM32H7x3 MCU can be measured in Standby mode. The ETH PHY is powered by VDD and is separated from VDD MCU. The silkscreen is corrected on the morpho connectors.</td><td rowspan="3">No limitation</td></tr><tr><td>MCU errata sheet:- STM32H750xB and STM32H753xI device limitations (ES0396)</td></tr><tr><td>Board:- MB1364-H753ZI-E01 (main board)</td></tr></table>


Table 23. Product history (continued)


<table><tr><td>Order code</td><td>Product identification</td><td>Product details</td><td>Product change description</td><td>Product limitations</td></tr><tr><td rowspan="3">NUCLEO-H753ZI</td><td rowspan="3">NUH753ZI$AT3</td><td>MCU:
STM32H753ZIT6
revision &#x27;V&#x27;</td><td rowspan="3">Packaging: plastic blister replaced by a carton box</td><td rowspan="3">No limitation</td></tr><tr><td>MCU errata sheet:
- STM32H750xB and
STM32H753x1 device
limitations (ES0396)</td></tr><tr><td>Board:
MB1364-H753ZI-E01
(main board)</td></tr><tr><td rowspan="6">NUCLEO-H723ZG</td><td rowspan="3">NUH723ZG$AT1</td><td>MCU:
- STM32H723ZGT6
revision &#x27;Z&#x27;</td><td rowspan="3">Initial revision</td><td rowspan="3">No limitation</td></tr><tr><td>MCU errata sheet:
- STM32H72xx/73xx
device errata (ES0491)</td></tr><tr><td>Board:
- MB1364-H723ZG-E01
(main board)</td></tr><tr><td rowspan="3">NUH723ZG$AT2</td><td>MCU:
- STM32H723ZGT6
revision &#x27;Z&#x27;</td><td rowspan="3">Packaging: plastic blister replaced by a carton box</td><td rowspan="3">No limitation</td></tr><tr><td>MCU errata sheet:
- STM32H72xx/73xx
device errata (ES0491)</td></tr><tr><td>Board:
- MB1364-H723ZG-E01
(main board)</td></tr></table>

# 10.3 Board revision history


Table 24. Board revision history


<table><tr><td>Board reference</td><td>Board variant and revision</td><td>Board change description</td><td>Board limitations</td></tr><tr><td rowspan="2">MB1364 (main board)</td><td>MB1364-H743ZI-C01
MB1364-H753ZI-C01</td><td>Initial revision</td><td>The IDD measurement of the STM32H7x3 MCU cannot be performed in Standby mode because of ETH PHY (U15) and the pull-up resistors (R40, R43, R46, R47, and R48) supplied by VDD MCU.</td></tr><tr><td>MB1364-H743ZI-E01
MB1364-H753ZI-E01
MB1364-H723ZG-E01</td><td>The IDD measurement of the STM32H7x3 MCU can be measured in Standby mode. The ETH PHY is powered by VDD and is separated from VDD MCU.
The silkscreen has been corrected on the morpho connectors.</td><td>No limitation</td></tr></table>

# 11 Compliance statements and conformity declarations

# 11.1 Federal Communications Commission (FCC) compliance statement

Part 15.19

These devices comply with part 15 of the FCC rules. Operation is subject to the following two conditions: (1) these devices may not cause harmful interference, and (2) these devices must accept any interference received, including interference that may cause undesired operation.

Part 15.21

Any changes or modifications to this equipment not expressly approved by STMicroelectronics may cause harmful interference and void the user's authority to operate this equipment.

Part 15.105

This equipment has been tested and found to comply with the limits for a Class B digital device, pursuant to part 15 of the FCC Rules. These limits are designed to provide reasonable protection against harmful interference in a residential installation. This equipment generates uses and can radiate radio frequency energy and, if not installed and used in accordance with the instructions, may cause harmful interference to radio communications. However, there is no guarantee that interference will not occur in a particular installation. If this equipment does cause harmful interference to radio or television reception which can be determined by turning the equipment off and on, the user is encouraged to try to correct interference by one or more of the following measures:

Reorient or relocate the receiving antenna.

- Increase the separation between the equipment and receiver.

- Connect the equipment into an outlet on a circuit different from that to which the receiver is connected.

- Consult the dealer or an experienced radio/TV technician for help.

Note: Use only shielded cables.

# Responsible party - U.S. contact information

Francesco Doddo

STMicroelectronics, Inc.

200 Summit Drive | Suite 405 | Burlington, MA 01803

USA

Telephone: +1 781-472-9634

# 11.2 Innovation, Science and Economic Development Canada (ISED) compliance statement

These products comply with the ICES-003 standard class B of the ISED regulation.

ISED Canada ICES-003 Compliance Label: CAN ICES (B)/NMB (B).

Note: Use only shielded cables.

Ces produits sont conformes à la norme NMB-003 classe B de la ISDE.

Étiquette de conformité à la NMB-003 d'ISDE Canada : CAN ICES (B) / NMB (B).

Note: Utiliser uniquement des cables blindés.

# 11.3 UKCA conformity

Simplified UK declaration of conformity

Hereby, the manufacturer STMicroelectronics, declares that the equipment types NUCLEO-H723ZG, NUCLEO-H743ZI, and NUCLEO-H753ZI are in compliance with the UK Electromagnetic Compatibility Regulations 2016 (UK SI 2016 No. 1091) and with the Restriction of the Use of Certain Hazardous Substances in Electrical and Electronic Equipment Regulations 2012 (UK SI 2012 No. 3032).

Note: Use only shielded cables.

# 11.4 CE conformity

# 11.4.1 Simplified EU declaration of conformity

Hereby, STMicroelectronics declares that the equipment types NUCLEO-H723ZG, NUCLEO-H743ZI, and NUCLEO-H753ZI are in compliance with directives 2011/53/EU and 2015/863/EU (RoHS), and 2014/30/EU (EMC).

Note: - RoHS: Restriction of hazardous substances

- EMC: Electromagnetic compatibility

Warning

These devices are compliant with Class B of EN55032/CISPR32. In a residential environment, this equipment may cause radio interference.

Note: Use only shielded cables.

# 11.4.2 Déclaration de conformité UE simplifiée

STMicroelectronics déclare que les équipements électriques des types NUCLEO-H723ZG, NUCLEO-H743ZI, et NUCLEO-H753ZI sont conformes aux directives 2011/53/UE et 2015/863/UE (LdSD), et à la directive 2014/30/UE (CEM).

Note: - LdSD : directive sur la limitation de l'utilisation des substances dangereuses

- CEM : compatibilité électromagnétique

# Avertissement

Ces équipements sont conformes à la Classe B de la EN55032 / CISPR 32. Dans un environnement résidentiel, ces équipements peuvent creer des interférences radio.

Note: Utiliser uniquement des cables blindés.

# 12 Product disposal

# Disposal of this product: WEEE (Waste Electrical and Electronic Equipment)

(Applicable in Europe)

![](images/647809c083d3a3f7bbf089d89ec05049ea78c96211b96186f44af96273c40218.jpg)


![](images/3343771456ab189cacb0f7c92035b1e6df1005fb589af7913e10c4dafaa9388c.jpg)


This symbol on the product, accessories, or accompanying documents indicates that the product and its electronic accessories must not be disposed of with household waste at the end of their working life.

To prevent possible harm to the environment and human health from uncontrolled waste disposal, separate these items from other types of waste and recycle them responsibly at a designated collection point to promote the sustainable reuse of material resources.

Household users:

Contact the retailer that you purchased the product from or your local authority for details of your nearest designated collection point.

Business users:

Contact your dealer or supplier for further information.

# Revision history


Table 25. Document revision history


<table><tr><td>Date</td><td>Revision</td><td>Changes</td></tr><tr><td>14-Mar-2019</td><td>1</td><td>Initial version</td></tr><tr><td>11-Jun-2020</td><td>2</td><td>Added:
- NUCLEO-H723ZG board
- Section 9 with Board revision history and Known limitations
Updated:
- Section 6.3 switch to STLINK-V3E
- Figure 1 to Figure 5, and Figure 9 to Figure 14</td></tr><tr><td>16-Jun-2023</td><td>3</td><td>Updated:
- Table 14 with SB16, SB17, SB81, and SB82 added configurations
- Table 20 with pin 15 corrected connection
- Nucleo-144 boards (MB1364) information
Removed:
- Direct firmware update in Embedded STLINK-V3E features
- Former Figure 15 and Figure 16 USB connectors
Removed the references to Arm® Mbed™.</td></tr><tr><td>12-Oct-2023</td><td>4</td><td>Added NUH723ZG$AT2 product identification to Table 23.</td></tr><tr><td>14-Oct-2025</td><td>5</td><td>Updated:
- Section 7.4: Power supply
- Section 7.5.1: HSE clock (high-speed external clock)
- Section 10.1: Product marking
- Table 23: Product history
- Section 11: Compliance statements and conformity declarations
Added:
- Section 3.4: EDA resources
- Section 5: Safety recommendations
- Section 12: Product disposal</td></tr></table>

# IMPORTANT NOTICE - READ CAREFULLY

STMicroelectronics NV and its subsidiaries ("ST") reserve the right to make changes, corrections, enhancements, modifications, and improvements to ST products and/or to this document at any time without notice.

In the event of any conflict between the provisions of this document and the provisions of any contractual arrangement in force between the purchasers and ST, the provisions of such contractual arrangement shall prevail.

The purchasers should obtain the latest relevant information on ST products before placing orders. ST products are sold pursuant to ST's terms and conditions of sale in place at the time of order acknowledgment.

The purchasers are solely responsible for the choice, selection, and use of ST products and ST assumes no liability for application assistance or the design of the purchasers' products.

No license, express or implied, to any intellectual property right is granted by ST herein.

Resale of ST products with provisions different from the information set forth herein shall void any warranty granted by ST for such product.

If the purchasers identify an ST product that meets their functional and performance requirements but that is not designated for the purchasers' market segment, the purchasers shall contact ST for more information.

ST and the ST logo are trademarks of ST. For additional information about ST trademarks, refer to www.st.com/trademarks. All other product or service names are the property of their respective owners.

Information in this document supersedes and replaces information previously supplied in any prior versions of this document.

© 2025 STMicroelectronics – All rights reserved
