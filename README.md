Some notes

----------

## MODULE 1 (WEB)
Simple web app which serves as a controls interface to interact with __MODULE 2 (TURRET)__. User can aim turret with mouse x,y and fire with LMB. Also displays video feed from __MODULE 2 (TURRET)__
IN: User input < from __USER__
OUT: User input > to __MODULE 2 (TURRET)__

### [Client]
Client facing portion, SPA. Let user authenticate. If user authenticated: send user input to server + receive video stream from server.
`JS: Angular2 framework`
#### Purposes:
- View feed from turret
- Interactive turret control
  - X,Y mouse movements (canvas mouse lock)
  - Set manual mode: controlled by user
  - ~~Set auto mode: controlled by __MODULE 3 (NEURAL NET)__~~ (later)
- Depending on auth
  - Public: No access (to save bandwidth)
  - Authorized: View and control

### [Static/API/WS Server]
Authenticate user. Guard API routes from unauthorized access. Receive and sanitize user input from client. Forward commands to __MODULE 2 (TURRET)__. Receive video stream from __MODULE 2 (TURRET)__ and upload to client.
`D: vibe.d HTTP/WS framework`
#### Purposes:
- Serve static content (compiled client bundles)
- Authentication 
  - Don't want randos staring at my backyard and controlling an airsoft turret
- Session management
- Sanitize user input commands received from client and forward to __MODULE 2 (TURRET)__
- Serve cam feed from __MODULE 2 (TURRET)__ (WebSocket?)

### [Database]
`MongoDB`
#### Purposes:
- Store user credentials
- Store sessions

----------

## MODULE 2 (TURRET)
Electronically controlled turret with mounted airsoft gun. 3D printed frame. Powered by Raspberry Pi + Arduino. Rotation controlled by 2 NEMA 17 stepper motors (may need something with more holding torque). Trigger switch actuated with relay. Receives control commands from __MODULE 1 (WEB)__ to control turret. ~~Receives control commands from either __MODULE 1 (WEB)__ or __MODULE 3 (CV / NEURAL NETWORK)__ to control turret (depending if manual or auto mode).~~
IN: User input < from __MODULE 1 (WEB)__
OUT: Video stream > to __MODULE 1 (WEB)__

### [Raspberry Pi]
Background application running on RPi which accepts input values from __MODULE 1 (WEB)__ via IPC. Sends commands to Arduino via USB serial connection in format defined below in Arduino section.
`Python`
#### Purposes: 
- Receive commands from __MODULE 1 (WEB)__
  - X,Y acceleration values
  - Fire
- Send commands to Arduino
- Send webcam feed to __MODULE 1 (WEB)__
- Act as “network bridge” between Arduino and __MODULE 1 (WEB)__
  - Cheaper, easier than connecting an Arduino to the internet.
  - May run into minor latency issues because not realtime OS

### [Arduino]
Interface for stepper motors, trigger relay. Interprets commands received from USB serial connection and performs appropriate action (e.g. rotate turret on ? axis in ? direction by ? degrees, fire). TODO: stepper motor controllers, battery config
`C++`
#### Purposes:
- Read 4-byte command from USB serial stream
- Control turrets
  - Stepper motor rotation
  - Firing
### Input Format/Protocol 
(assuming reliable multi-byte serial stream - order of bytes sent preserved):
Input Serial Stream: 32-bit wide binary string
Masks:
0x0000 = EMPTY PADDING (START OF COMMAND)
0x0F00 = ROTATE X (-128, 127)
0x00F0 = ROTATE Y (-128, 127)
0x000F = TRIGGER STATE (1, 255) (END OF COMMAND)
First byte is ignored, only used to determine start of 32-bit binary string
Second byte cast to signed 8-bit integer as X axis desired rotation
Third byte cast to signed 8-bit integer as Y axis desired rotation
Fourth byte cast to unsigned 8-bit integer as trigger state where 1 = off, 255 = on. Must not be zero.

----------

##MODULE 3 (CV/NEURAL NET) [Later]

### [Service]
`C++/Python: OpenCV framework`?
#### Purposes:
- Automatic motion detection
- Send commands (rotate, fire) to Module 2 Raspberry Pi
