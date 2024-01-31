# Example for VCI4 FSL Library (Frame & Signal Programming Library)

## About FSL Library (Frame & Signal Programming Library)

The FSL Library simplifies programming VCI Applications which wants to deal with CAN networks 
on a higher abstraction level. 
With FSL the description of the communication primitives like Frames, FrameTriggerings and Signals
reside in a database. The FSL library could load such databases and provide an access to the
application specific signals. 
Both reading and writing signal values is supported through different types of signal sets.

## Features

FSL supports the import of various database formats

  - CANDB (Vector Informatic tools)
  - DIMPRJ (Ixxat DIM database format, Data interpreter Module) used in Ixxat canAnalyser
  - FIBEX (Fieldbus exchange format, Asam standard)
  - LDF (LIN description file, LIN standard)
  
## Sample application

The sample application loads the FIBEX database Weatherstation.xml and 

  - creates an IRSignalSet for reading a given set of signals
    Pressing key "r" reads and prints the content of this set.
  - creates an ITSignalSet for writing a given set of signals
    Pressing key "t" writes an incremented value to the signals of this set

Frames that are selected by the ITSignalSet instance are transmitted according to their
settings in the database. This includes cyclic transmission where cycle times are specified.

The sample tries to load the FIBEX database Weatherstation.xml from 
  - .
  - ../../common
  - ./common

### Usage

#### Transmit messages

After starting the sample application you have to select a CAN device and the CAN controller
on this device.
This controller is initialized by the sample application to a CAN bitrate of 1000kBit/s.
After that the signal sets are created and the transmit signal set is enabled.
You can connect to the CAN network via a second CAN controller e.g. with the canAnalyser 
or canAnalyserMini and you will see the following frames transmitted cyclically:

            ID      Data
            66      00 00 00 00 00                  ; Frame Humidity
            65      00 00 00 00                     ; Frame Pressure
            67      00 00 00 00 00 00 00 00         ; Frame Wind
            69      00 00 00 00                     ; Frame Radiation
            68      00 00 00 00                     ; Frame Rainfall
            64      00 00 00 00                     ; Frame Temperature
            67      00 00 00 00 00 00 00 00         ; Frame Wind

The measured cycle times should be slightly below the cycle times specified in the database.

The pdu within the frame Humidity contains two signals

  - Air: length 8 bit, located in byte 0, encoded as uint8 
  - DewPoint: length 32 bit, located in byte 1 to 4, encoded as float32

After pressing "t" the signals Air and DewPoint in the frame has been changed

            ID      Data
            66      52 F1 92 27 C2                  ; Frame Humidity
                                                    ; Air = 82% (raw: 0x52) 
                                                    ; DewPoint = -41.893497467041 °C (raw: 0xc22792f1)

The new values are created inside the sample application from random offsets,
so the actual values on your side could differ.
Every further press on "t" does change the signals in the transmit set.

                                                   
#### Receive messages
  
By pressing "r" you get a dump of the receive signal set:

            Air Pressure    [hPa] : VT SINGLE | rx-count: 0 | Value: 0
            Air Temperature [C]   : VT SINGLE | rx-count: 0 | Value: 0
            Air Humidity    [%]   : VT UINT8  | rx-count: 0 | Value: 00
            Dew Point       [C]   : VT SINGLE | rx-count: 0 | Value: 0
            Solar Radiation [W/m2]: VT SINGLE | rx-count: 0 | Value: 0
            Wind Velocity   [km/h]: VT SINGLE | rx-count: 0 | Value: 0
            Wind Direction  [deg] : VT UINT16 | rx-count: 0 | Value: 0000
            Hourly Rainfall [mm/h]: VT UINT16 | rx-count: 0 | Value: 0000
            Daily Rainfall  [mm/d]: VT UINT16 | rx-count: 0 | Value: 0000
               
which is empty because no corresponding messages have been received via the CAN bus.
Sending 

            ID      Data
            67      FF 02 03 3E 11 22 00 00         ; Frame Wind

changes the Direction and the Speed signal because they both reside in the 
FrameTriggering with ID 67hex.
Speed is a 32 bit float value located in byte 0 to 3.
Direction is a 16 bit unsigned int value located in byte 4 to 5.
  
            Wind Velocity   [km/h]: VT DOUBLE | rx-count:   1 | Value: 0.127941
            Wind Direction  [deg] : VT UINT64 | rx-count:   1 | Value: 0000000000002211

Sending
               
            ID      Data
            67      00 00 C8 42 B4 00 00 00
  
sets the Direction to B4hex = 180 and Speed to 100 km/h
  
            Wind Velocity   [km/h]: VT DOUBLE | rx-count: 1 | Value: 100
            Wind Direction  [deg] : VT DOUBLE | rx-count: 1 | Value: 180
