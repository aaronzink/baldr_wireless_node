# baldr_wireless_node
Baldr wireless node software derived from the Microchip MiWi Demo Kit software.

Steps to get started;

Setup:
1. Install the MPLAB X IDE
	* http://www.microchip.com/mplab/mplab-x-ide
2. Install the free XC8 Compiler
	* http://www.microchip.com/mplab/compilers
3. Download and install the pickit 2 programmer (v2.60)
	* ww1.microchip.com/downloads/en/DeviceDoc/PICkit%202%20v2.60.00%20Setup%20A.zip
4. Checkout the source code

Using the MPLAB IDE
1. Open the project folder/file in MPLAB IDE
  * baldr_wireless_node/apps/miwi/miwi_mesh/miwi_demo_kit/firmware/baldr_wireless_node.X
2. Use the top bar of the IDE to choose the _89xa transciver
3. Build the project (click the hammer, or run>build project)

Using the picket 2 programmer
1. Open the picket 2 programmer
2. Make sure that the board is on and the programmer is connected to the board
3. Select Tools>Check Communication and you should see "PICkit 2 found and connected" and "PIC Device Found"
4. In the picket 2 programmer select our PIC device
	* Device Family>PIC18F_J_
5. Program the hex file to the demo board
	* Select File>Import Hex
	* Navigate to /apps/miwi/miwi_mesh/miwi_demo_kit/firmware/baldr_wireless_node.X/dist/miwikit_pic18f46j50_89xa/production
6. Click the "Write" button

