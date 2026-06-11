<table border="0">
  <tr>
    <td align="left" valign="middle">
	  <h1>Simplicity SDK<br/>32-bit MCU Peripheral Examples</h1>
	</td>
	<td align="left" valign="middle">
	  <a href="https://www.silabs.com/products/mcu/32-bit">
	    <img src="http://pages.silabs.com/rs/634-SLU-379/images/WGX-transparent.png"  title="Silicon Labs Gecko and Wireless Gecko MCUs" alt="EFM32 32-bit Microcontrollers" width="250"/>
	  </a>
	</td>
  </tr>
</table>

This repo contains simple peripheral examples based on emlib in Simplicity SDK for Series 2 devices.

## Supported Series 2 Radio Boards and Devices ##
<table>
  <thead>
    <tr>
      <th style="text-align:left; padding:8px 40px;">Board ID</th>
      <th style="text-align:left; padding:8px 40px;">Device</th>
    </tr>
  </thead>
  <tbody>
    <tr><td style="padding:8px 40px;">BRD4181A</td><td style="padding:8px 40px;">EFR32MG21</td></tr>
    <tr><td style="padding:8px 40px;">BRD4184B</td><td style="padding:8px 40px;">EFR32BG22</td></tr>
    <tr><td style="padding:8px 40px;">BRD4182A</td><td style="padding:8px 40px;">EFR32MG22</td></tr>
    <tr><td style="padding:8px 40px;">BRD4204D</td><td style="padding:8px 40px;">EFR32ZG23</td></tr>
    <tr><td style="padding:8px 40px;">BRD4186C</td><td style="padding:8px 40px;">EFR32MG24</td></tr>
    <tr><td style="padding:8px 40px;">BRD4270B</td><td style="padding:8px 40px;">EFR32FG25</td></tr>
    <tr><td style="padding:8px 40px;">BRD4117A</td><td style="padding:8px 40px;">EFR32MG26</td></tr>
    <tr><td style="padding:8px 40px;">BRD4111A</td><td style="padding:8px 40px;">EFR32BG27</td></tr>
    <tr><td style="padding:8px 40px;">BRD2602A</td><td style="padding:8px 40px;">EFR32BG27</td></tr>
    <tr><td style="padding:8px 40px;">BRD4194A</td><td style="padding:8px 40px;">EFR32MG27</td></tr>
    <tr><td style="padding:8px 40px;">BRD4400C</td><td style="padding:8px 40px;">EFR32ZG28</td></tr>
    <tr><td style="padding:8px 40px;">BRD4420A</td><td style="padding:8px 40px;">EFR32BG29</td></tr>
    <tr><td style="padding:8px 40px;">BRD4412A</td><td style="padding:8px 40px;">EFR32MG29</td></tr>
  </tbody>
</table>

## Supported Series 3 Radio Board and Device ##
<table>
  <thead>
    <tr>
      <th style="text-align:left; padding:8px 40px;">Board ID</th>
      <th style="text-align:left; padding:8px 40px;">Device</th>
    </tr>
  </thead>
  <tbody>
    <tr><td style="padding:8px 40px;">BRD4407A</td><td style="padding:8px 40px;">SiMG301</td></tr>
  </tbody>
</table>

## Requirements ##
1. A compatible **Silicon Labs Starter Kit**
2. **Simplicity Studio 6**
3. **Simplicity SDK** (match the SDK version noted in the commit tag)
4. Clone this repository to a local directory (any location is supported), for example:`C:\Users\<username>\SimplicityStudio\extensions` 

## Installing the SDK Extension 

In Simplicity Studio 6, extensions must be installed through the Studio and attached to a specific SDK.

1. Open **Settings**
2. Navigate to **SDKs**
3. Locate your installed **Simplicity SDK**
4. Click **Add Extension**
5. Browse to the folder where this repository was cloned
6. Select the extension and click **Finish**
7. When prompted, click **Trust**

## Importing Examples into Simplicity Studio 6 ##

1. Launch **Simplicity Studio 6**
2. Open the Devices view by clicking the **DEVICES** button
3. Select your development board:
   - from the **CONNECTED** section (if hardware is attached), or
   - from the **VIRTUAL** section (if no hardware is connected)
4. To locate your device, use:
   - the **search bar**, or
   - filters such as **Technology**, **Device Type**, or **Board**
5. Click on your selected device to open its device view
6. In the device view, click **Create New Project**
7. In the **Example Projects & Demos** page:
   - browse or filter available examples
   - select an example
   - click **CREATE**  
8. Complete project configuration and click **Finish** to import the example into your workspace

## Adding Support for a New Board
These peripheral examples use the `peripheral_examples_evaluation_templates.xml` file along with a custom component that provides dedicated Pin Configuration headers to support multiple development boards. To add support for a new board:

### 1. Update Compatibility Metadata
Modify the `partCompatibility` and `boardCompatibility` entries for the example inside: `peripheral_examples_evaluation_templates.xml`

### 2. Add a Pin Configuration File
Create a new `pin_config.h` file for your board and place it under the appropriate kit directory, following the structure used by existing boards
Example: `series2/kit/EFR32MG21_BRD4181A/pin_config.h`

### 3. Update `silabs_bsp.slcc`
Append the new pin configuration path:
```yaml
- override:
    component: "%extension-peripheral_examples%silabs_bsp"
- path: series2/kit/EFR32MG21_BRD4181A/pin_config.h
  condition: [brd4181a]
  ```

### 4. Refresh the SDK
After installing the extension:
1. Open **Window → Preferences**
2. Navigate to **Simplicity Studio → SDKs**
3. Click **Refresh**


## ⚠️ Compatibility Warning

SiXG301 is **not supported** when using the IAR toolchain **v9.40.1**, which is the latest version officially supported by the Simplicity SDK noted in this repository's commit tag. Refer to the Simplicity SDK release notes for more information.

## Reporting Bugs/Issues and Posting Questions and Comments ##

All examples in this repo is are considered EVALUATION QUALITY, meaning this code has been minimally tested to ensure that it builds with the specified dependencies and is suitable as a demonstration for evaluation purposes only. This code will be maintained at the sole discretion of Silicon Labs.

To report bugs in the peripheral example projects, please create a new "Issue" in the "Issues" section of this repo.  Please reference the board, project, and source files associated with the bug, and reference line numbers.  If you are proposing a fix, also include information on the proposed fix.  Silicon Labs engineers will address bugs and push them to the public repository periodically.

Questions and comments related to the peripheral examples should be made by creating a new "Issue" in the "Issues" section of this repo.
