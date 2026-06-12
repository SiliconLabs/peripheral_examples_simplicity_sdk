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
    <tr><td style="padding:8px 40px;">BRD4277A</td><td style="padding:8px 40px;">EFR32FG2D</td></tr>
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
3. **Simplicity SDK** (match the SDK version noted in the commit tag, if cloning directly)

## Obtaining the Peripheral Examples
There are two ways to obtain the peripheral examples:

### Option 1 (Recommended): Install via Simplicity Installer/SLT
The peripheral examples are available as an installable extension directly within **Simplicity Installer/SLT**, making this the easiest way to get started (no repository cloning required).
> The version of the peripheral examples installed through Simplicity Studio is automatically aligned with your installed **Simplicity SDK**. 

#### Using Simplicity Installer (GUI)
1. Download **Simplicity Installer** for your OS  
2. Open **Simplicity Installer**
3. Click on **Installation Wizard**
4. Click **Technology Install**
5. Select **Peripheral Examples** under *Optional Packages*
6. Click **NEXT**
7. Accept the Terms of Use and License Agreement
8. Click **INSTALL** to begin installation

#### Using Silicon Labs Tool (SLT CLI)
1. Download **Silicon Labs Tool (SLT)** for your OS  
2. Extract the archive and add the directory to your system `PATH`
3. Install the SDK (this will also make associated extensions available):
```bash
slt install simplicity-sdk
```

### Option 2: Clone from GitHub (Manual Installation)
> ⚠️ **Important:**  
> You must clone or check out the **repository tag/commit that matches your Simplicity SDK version** to ensure compatibility.

```bash
git clone https://github.com/SiliconLabsSoftware/peripheral_examples.git
cd peripheral_examples
git checkout <sdk-matching-tag>
```

Then install it as an SDK extension:
1. Open **Simplicity Studio 6 → Settings**  
2. Navigate to **SDKs**  
3. Select your installed **Simplicity SDK**  
4. Click **Add Extension**  
5. Browse to the cloned repository folder  
6. Select the extension and click **Finish**  
7. When prompted, click **Trust**

## Post-Installation Steps
After successful installation:
1. Restart Simplicity Studio to ensure the extension is properly loaded.
2. Verify Installation, open **settings → SDKs**  and confirm that **Peripheral Examples** appears as extension under SDK 
3. Explore peripheral examples that are available in the **Example Projects & Demos** view when creating a new project.

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

### 2. Add Configuration Files
Create a new `pin_config.h` file and a new `peripheral_config.h` file for your board and place it under the appropriate directory, following the structure used by existing boards
Example: `components/series2/EFR32MG21_BRD4181A/pin_config.h`

### 3. Update `silabs_bsp.slcc` and `silabs_peripheral_config.slcc` files
Append the new configuration path. 

For example:
```yaml
- override:
    component: "%extension-peripheral_examples%silabs_bsp"
- path: components/series2/EFR32MG21_BRD4181A/pin_config.h
  condition: [brd4181a]
  ```

### 4. Refresh the SDK
1. Open **settings → SDKs**
2. Select the SDK version used and click **Refresh**

## Reporting Bugs/Issues and Posting Questions and Comments ##

All examples in this repo is are considered EVALUATION QUALITY, meaning this code has been minimally tested to ensure that it builds with the specified dependencies and is suitable as a demonstration for evaluation purposes only. This code will be maintained at the sole discretion of Silicon Labs.

To report bugs in the peripheral example projects, please create a new "Issue" in the "Issues" section of this repo (https://github.com/SiliconLabsSoftware/peripheral_examples).  Please reference the board, project, and source files associated with the bug, and reference line numbers.  If you are proposing a fix, also include information on the proposed fix.  Silicon Labs engineers will address bugs and push them to the public repository periodically.

Questions and comments related to the peripheral examples should be made by creating a new "Issue" in the "Issues" section of this repo.
