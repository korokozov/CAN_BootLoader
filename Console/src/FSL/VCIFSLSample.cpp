//////////////////////////////////////////////////////////////////////////
// HMS Technology Center Ravensburg GmbH
//////////////////////////////////////////////////////////////////////////
/**

  Demo application for the IXXAT VCI-FSL-API.

  @note 
    This demo demonstrates the following VCI features
    - controller selection
    - controller initialization 
    - creation of a message channel
    - transmission / reception of CAN messages

*/
//////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016-2019
// HMS Technology Center Ravensburg GmbH, all rights reserved
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////////

#include "vcisdk.h"
#include "vcifsl.h"

#include <tchar.h>
#include <process.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <assert.h>

#include "SocketSelectDlg.hpp"

//////////////////////////////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////////////////////////////

static LONG          lBusCtl = -1;    // controller number
static IBalObject*   pBalObj = NULL;  // bus access object
static ICanControl2* pCanCtl = NULL;  // CAN control interface

//
// Signal database
//

// Select the database to use:
// #define USE_CANDB_DATABASE
#define USE_FIBEX_DATABASE

// CANDB does not support clusters and channels, but the
// import inserts dummy nodes defaultCluster and defaultChannel
#ifdef USE_CANDB_DATABASE
#define SIGNAL_DB_PATH   TEXT("Weatherstation.dbc")
#define SIGNAL_DB_PARAMS TEXT("cluster=defaultCluster;channel=defaultChannel")
#endif

// FIBEX supports clusters and channels
// see the database file for cluster "CAN1" and channel "Channel1"
#ifdef USE_FIBEX_DATABASE
#define SIGNAL_DB_PATH   TEXT("Weatherstation.xml")
#define SIGNAL_DB_PARAMS TEXT("cluster=CAN1;channel=Channel1")
#endif

//
// Signals selected from database
//
// Signals are specified by the short name of either Frames or PDUs
// ($frm/ or $pdu/) appended by a slash and the signal short name.
// If "$frm/" or "$pdu/" is omitted the LoadDB function expects a
// frame short name appended by a slash and the signal short name.
// See Syntax description below.
//
// Syntax of the Signal strings:
//
// "<Frame Short Name>/<Signal Short Name>"      -> database lookup by frame
// "$frm/<Frame Short Name>/<Signal Short Name>" -> database lookup by frame
// "$pdu/<PDU Short Name>/<Signal Short Name>"   -> database lookup by PDU
//
// Alternatively the signal can be also specifieyd by the database internal
// signal ID. In this case the signal is selected by a string in the following
// form: "$id/<signal id>"
// 
// The following example selects the signal with the database internal ID
// "sig4" within the example database:
//
//  TEXT("$id/sig4\0")
//
// This signal selects the same signal as the following strings:
//
//  TEXT("$frm/Wind/Speed\0") or TEXT("$pdu/Wind_pdu/Speed\0")
//

static TCHAR szSignals[] =
  TEXT("Pressure/Air\0")               // Frame: 'Pressure'      Signal: 'Air'
  TEXT("$frm/Temperature/Air\0")       // Frame: 'Temperature'   Signal: 'Air'
  TEXT("$frm/Humidity/Air\0")          // Frame: 'Humidity'      Signal: 'Air'
  TEXT("$pdu/Humidity_pdu/DewPoint\0") // PDU  : 'Humidity_pdu'  Signal: 'DewPoint'
  TEXT("$pdu/Radiation_pdu/Solar\0")   // PDU  : 'Radiation_pdu' Signal: 'Solar'
  TEXT("$pdu/Wind_pdu/Speed\0")        // PDU  : 'Wind_pdu'      Signal: 'Speed'
  TEXT("$pdu/Wind_pdu/Direction\0")    // PDU  : 'Wind_pdu'      Signal: 'Direction'
  TEXT("$pdu/Rainfall_pdu/Hourly\0")   // PDU  : 'Rainfall_pdu'  Signal: 'Hourly'
  TEXT("$frm/Rainfall/Daily\0")        // Frame: 'Rainfall'      Signal: 'Daily'
  TEXT("\0");                          // empty string marks end of signal list

#define NUM_SIGNALS 9

static PTSTR szSignalNames[NUM_SIGNALS] =
{
  TEXT("Air Pressure    [hPa] "),
  TEXT("Air Temperature [C]   "),
  TEXT("Air Humidity    [%]   "),
  TEXT("Dew Point       [C]   "),
  TEXT("Solar Radiation [W/m2]"),
  TEXT("Wind Velocity   [km/h]"),
  TEXT("Wind Direction  [deg] "),
  TEXT("Hourly Rainfall [mm/h]"),
  TEXT("Daily Rainfall  [mm/d]")
};

#define SIG_NAMES_FMT_STR TEXT("\n%s: ")

#define SIG_IDX_PRESSURE_AIR       0
#define SIG_IDX_TEMPERATURE_AIR    1
#define SIG_IDX_HUMIDITY_AIR       2
#define SIG_IDX_HUMIDITY_DEWPOINT  3
#define SIG_IDX_RADIATION_SOLAR    4
#define SIG_IDX_WIND_VELOCITY      5
#define SIG_IDX_WIND_DIRECTION     6
#define SIG_IDX_RAINFALL_HOURLY    7
#define SIG_IDX_RAINFALL_DAILY     8

//
// receive signal set
//
static IRSignalSet* pRxSignalSet = NULL;
static HANDLE       ahRxSignals[NUM_SIGNALS];

//
// receive signal buffers
//

static FSLSIGNAL aRxSignal[NUM_SIGNALS] =
{
  NULL, 0, 0, {FSL_VT_SINGLE, 0,0,0,0}, // SIG_IDX_PRESSURE_AIR
  NULL, 0, 0, {FSL_VT_SINGLE, 0,0,0,0}, // SIG_IDX_TEMPERATURE_AIR
  NULL, 0, 0, {FSL_VT_UINT8,  0,0,0,0}, // SIG_IDX_HUMIDITY_AIR
  NULL, 0, 0, {FSL_VT_SINGLE, 0,0,0,0}, // SIG_IDX_HUMIDITY_DEWPOINT
  NULL, 0, 0, {FSL_VT_SINGLE, 0,0,0,0}, // SIG_IDX_RADIATION_SOLAR
  NULL, 0, 0, {FSL_VT_SINGLE, 0,0,0,0}, // SIG_IDX_WIND_VELOCITY
  NULL, 0, 0, {FSL_VT_UINT16, 0,0,0,0}, // SIG_IDX_WIND_DIRECTION
  NULL, 0, 0, {FSL_VT_UINT16, 0,0,0,0}, // SIG_IDX_RAINFALL_HOURLY
  NULL, 0, 0, {FSL_VT_UINT16, 0,0,0,0}  // SIG_IDX_RAINFALL_DAILY
};

//
// transmit signal set
//
#define TRANSMIT_SIGS
#ifdef TRANSMIT_SIGS
static ITSignalSet* pTxSignalSet = NULL;
static HANDLE       ahTxSignals[NUM_SIGNALS];

//
// transmit signal buffers
//
static FSLSIGNAL aTxSignal[NUM_SIGNALS] =
{
  NULL, 0, 0, {FSL_VT_UINT8, 0,0,0,0}, // SIG_IDX_PRESSURE_AIR
  NULL, 0, 0, {FSL_VT_UINT8, 0,0,0,0}, // SIG_IDX_TEMPERATURE_AIR
  NULL, 0, 0, {FSL_VT_UINT8, 0,0,0,0}, // SIG_IDX_HUMIDITY_AIR
  NULL, 0, 0, {FSL_VT_UINT8, 0,0,0,0}, // SIG_IDX_HUMIDITY_DEWPOINT
  NULL, 0, 0, {FSL_VT_UINT8, 0,0,0,0}, // SIG_IDX_RADIATION_SOLAR
  NULL, 0, 0, {FSL_VT_UINT8, 0,0,0,0}, // SIG_IDX_WIND_VELOCITY
  NULL, 0, 0, {FSL_VT_UINT8, 0,0,0,0}, // SIG_IDX_WIND_DIRECTION
  NULL, 0, 0, {FSL_VT_UINT8, 0,0,0,0}, // SIG_IDX_RAINFALL_HOURLY
  NULL, 0, 0, {FSL_VT_UINT8, 0,0,0,0}  // SIG_IDX_RAINFALL_DAILY
};

static BOOL8 afTxReady[NUM_SIGNALS];
static BOOL8 afTxDone[NUM_SIGNALS];

#endif // TRANSMIT_SIGS


//////////////////////////////////////////////////////////////////////////
// function prototypes
//////////////////////////////////////////////////////////////////////////

HRESULT SelectDevice ( BOOL fUserSelect );
HRESULT CheckFeatures( void );
void    StartControl ( void );
void    FinalizeApp  ( void );

void    ReceiveData  ( void );
void    TransmitData ( void );

//////////////////////////////////////////////////////////////////////////
// locate signal DB
//////////////////////////////////////////////////////////////////////////

/* Return TRUE if file 'fileName' exists */
bool FileExists(const TCHAR *fileName)
{
  DWORD       fileAttr;

  fileAttr = GetFileAttributes(fileName);
  if (0xFFFFFFFF == fileAttr)
    return false;
  return true;
}

HRESULT LocateSignalDb(TCHAR *fileName, size_t bufsize)
{
  // search in current directory
  _tcscpy_s(fileName, bufsize, TEXT(""));
  _tcscat_s(fileName, bufsize, SIGNAL_DB_PATH);
  if (FileExists(fileName))
    return S_OK;

  // started in  subdirectory
  _tcscpy_s(fileName, bufsize, TEXT("..\\..\\..\\src\\FSL\\"));
  _tcscat_s(fileName, bufsize, SIGNAL_DB_PATH);
  if (FileExists(fileName))
    return S_OK;

  // debugging scenario
  _tcscpy_s(fileName, bufsize, TEXT(".\\FSL\\"));
  _tcscat_s(fileName, bufsize, SIGNAL_DB_PATH);
  if (FileExists(fileName))
    return S_OK;

  return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

//////////////////////////////////////////////////////////////////////////
/**
  Main entry point of the application.
*/
//////////////////////////////////////////////////////////////////////////
void main(void)
{
  HRESULT hResult;
  printf("\n# CAN-Loader started... Find CAN_USB Device\n");

  hResult = SelectDevice(FALSE);

  if (hResult == VCI_OK)
  {
    //
    // Create and initialize a CAN message switch
    //
    if (hResult == VCI_OK)
    {
      ICanMsgSwitch* pMsgSwitch = NULL;

      _tprintf(TEXT("\n Create CAN message switch..."));

      hResult = VciCreateCanMsgSwitch(pBalObj, lBusCtl, &pMsgSwitch);

      if (hResult == VCI_OK)
      {
        _tprintf(TEXT("OK!"));
        _tprintf(TEXT("\n Initialize CAN message switch..."));

        hResult = pMsgSwitch->Initialize(
          1,                      // Scheduling time of the message distributor
                                  // thread (in milliseconds). The value specified 
                                  // by this parameter must be less than the minimal
                                  // values specified for the cycle or delay time of
                                  // the transmit clients.
          THREAD_PRIORITY_NORMAL, // Priority of the message distributor thread.
          FALSE,                  // If this parameter is set to TRUE the
                                  // function tries to create the CAN message
                                  // channel in exclusive mode, otherwise the
                                  // function creates a shareable CAN channel.
          0x256,                  // Size of the receive FIFO in number of CAN
                                  // messages
          0x256 );                // Size of the transmit FIFO in number of CAN
                                  // messages

        if (hResult == VCI_OK)
        {
          _tprintf(TEXT(" OK!"));

          // try to start the CAN controller connected with the message switch

          _tprintf(TEXT("\n Try to acquire CAN controller..."));
          if (pMsgSwitch->GetControl(&pCanCtl) == VCI_OK)
          {
            _tprintf(TEXT("OK!"));
            StartControl();
          }
          else
          {
            _tprintf(TEXT("busy!"));
          }
        }
        else
        {
          _tprintf(TEXT("failed ! Error=0x%08X"), hResult);
        }
      }
      else
      {
        _tprintf(TEXT("failed ! Error=0x%08X"), hResult);
      }

      TCHAR signalDbPath[2 * MAX_PATH];
      hResult = LocateSignalDb(signalDbPath, ARRAYSIZE(signalDbPath));
      if (hResult != S_OK)
      {
        _tprintf(TEXT("SignalDb not found ! Error=0x%08X"), hResult);
      }

      //
      // create Receive Signal Set
      //
      if (hResult == VCI_OK)
      {
        _tprintf(TEXT("\n Create Receive Signal Set..."));

        hResult = pMsgSwitch->CreateClient(
          IID_IRSignalSet, (PVOID*) &pRxSignalSet);

        if (hResult == VCI_OK)
        {
          _tprintf(TEXT("OK!"));
          _tprintf(TEXT("\n Load Receive Signal Set..."));

          hResult = pRxSignalSet->LoadDB(
            signalDbPath,     // Pointer to the pathname of the database file.
            SIGNAL_DB_PARAMS, // Pointer to an null-terminated string specifying
                              // additional parameters required to access the
                              // database.
            szSignals,        // Pointer to a buffer containing NULL-terminated
                              // strings with the names of the signals to load.
                              // The last string in the buffer must be an empty
                              // string ("\0").
            NULL,             // Optional array of values which specifies the
                              // depth of the buffer for the specified signals.
                              // If a value within this array is greater than 1,
                              // the function uses a FIFO, otherwise a single
                              // buffer is used. If this parameter is set to
                              // NULL, the function uses a single buffer for all
                              // signals. If the pointer value of this parameter
                              // is less than 65536, the pointer value specifies
                              // the buffer depth used for all signal buffers
                              // within the set. If this parameter points to an
                              // array, this array must be of the same size as
                              // the array for the signal IDs.
            ahRxSignals,      // Pointer to an array where the function stores
                              // the IDs of the loaded signals. If a signal name
                              // specified within <szRxSignals> is not found
                              // within the database, the function sets the
                              // corresponding array entry to NULL.
            NUM_SIGNALS);     // Size of the arrays pointed to by <ahRxSignals>

          if (VCI_OK == hResult )
          {
            _tprintf(TEXT("OK!"));

            // check signal handles
            for (int i = 0; i < NUM_SIGNALS; ++i)
            {
              // handle is 0 if lookup has failed
              assert(0 != ahRxSignals[i]);

              // initialize RX signal buffers
              aRxSignal[i].hSigId = ahRxSignals[i];
            }

            _tprintf(TEXT("\n Enable Receive Signal Set..."));
            hResult = pRxSignalSet->Enable();
            if (VCI_OK == hResult )
              _tprintf(TEXT("OK!"));
            else
              _tprintf(TEXT("failed! Error=0x%08X"), hResult);
          }
          else
          {
            _tprintf(TEXT("failed! Error=0x%08X"), hResult);
          }
        }
        else
        {
          _tprintf(TEXT("failed! Error=0x%08X"), hResult);
        }

      }

      #ifdef TRANSMIT_SIGS
      //
      // create the Transmit Signal Set
      //
      if (hResult == VCI_OK)
      {
        _tprintf(TEXT("\n Create Transmit Signal Set..."));

        hResult = pMsgSwitch->CreateClient(
          IID_ITSignalSet, (PVOID*) &pTxSignalSet);

        if ( VCI_OK == hResult )
        {
          _tprintf(TEXT("OK!"));
          _tprintf(TEXT("\n Load Transmit Signal Set..."));

          hResult = pTxSignalSet->LoadDB(
            signalDbPath,     // Pointer to the pathname of the database file.
            SIGNAL_DB_PARAMS, // Pointer to an null-terminated string specifying
                              // additional parameters required to access the
                              // database.
            szSignals,        // Pointer to a buffer containing NULL-terminated
                              // strings with the names of the signals to load.
                              // The last string in the buffer must be an empty
                              // string ("\0").
            NULL,             // Optional array of values which specifies the
                              // depth of the buffer for the specified signals.
                              // If a value within this array is greater than 1,
                              // the function uses a FIFO, otherwise a single
                              // buffer is used. If this parameter is set to
                              // NULL, the function uses a single buffer for all
                              // signals. If the pointer value of this parameter
                              // is less than 65536, the pointer value specifies
                              // the buffer depth used for all signal buffers
                              // within the set. If this parameter points to an
                              // array, this array must be of the same size as
                              // the array for the signal IDs.
            ahTxSignals,      // Pointer to an array where the function stores
                              // the IDs of the loaded signals. If a signal name
                              // specified within <szRxSignals> is not found
                              // within the database, the function sets the
                              // corresponding array entry to NULL.
            NUM_SIGNALS);     // Size of the arrays pointed to by <ahRxSignals>


          if (hResult == VCI_OK)
          {
            _tprintf(TEXT("OK!"));
            // check signal handles
            for (int i = 0; i < NUM_SIGNALS; ++i)
            {
              //handle is NULL if signal lookup has failed
              //assert(ahTxSignals[i] != NULL);

              // initialize TX signal buffers
              aTxSignal[i].hSigId = ahTxSignals[i];

              // TX-ready and TX-done buffers
              afTxReady[i] = FALSE;
              afTxDone[i]  = FALSE;
            }

            _tprintf(TEXT("\n Enable Transmit Signal Set..."));
            hResult = pTxSignalSet->Enable();
            if (VCI_OK == hResult )
              _tprintf(TEXT("OK!"));
            else
              _tprintf(TEXT("failed! Error=0x%08X"), hResult);
          }
          else
          {
            _tprintf(TEXT("failed! Error=0x%08X"), hResult);
          }
        }
      }
      #endif //TRANSMIT_SIGS

      //
      // activate the CAN message switch
      //
      if (hResult == VCI_OK)
      {
        _tprintf(TEXT("\n Activate message switch..."));
        hResult = pMsgSwitch->Activate();
        if (VCI_OK == hResult )
          _tprintf(TEXT("OK!"));
        else
          _tprintf(TEXT("failed! Error=0x%08X"), hResult);
      }

      //
      // process user input
      //
      if (hResult == VCI_OK)
      {
        int iKey = 0;

        printf("\nTerminal ready\n");

        do
        {
          // wait for the user to press a key
          iKey = _getch();

          switch (iKey)
          {
            case 'R':
            case 'r':
              ReceiveData();
              break;

            case 'T':
            case 't':
              TransmitData();
              ReceiveData();
              break;

            case VK_ESCAPE:
              break;
          }
        }
        while (iKey != VK_ESCAPE);

        //
        // deactivate the CAN message switch
        //
        pMsgSwitch->Deactivate();
        // after deactivation the CAN message swicht does
        // no longer receive or transmit CAN messages

        //
        // terminate CAN message switch
        //
        if (pRxSignalSet != NULL)
        {
          pRxSignalSet->Disable();
          pRxSignalSet->CloseDB();
          pRxSignalSet->Release();
          pRxSignalSet = NULL;
        }

        #ifdef TRANSMIT_SIGS
        if (pTxSignalSet != NULL)
        {
          pTxSignalSet->Disable();
          pTxSignalSet->CloseDB();
          pTxSignalSet->Release();
          pTxSignalSet = NULL;
        }
        #endif //TRANSMIT_SIGS
      }

      pMsgSwitch->Release();
      pMsgSwitch = NULL;
    }
  }

  _tprintf(TEXT("\n\n Terminate application..."));
  FinalizeApp();
  _tprintf(TEXT("\n Goodbye!!\n\n"));
}

//////////////////////////////////////////////////////////////////////////
/**
  Select CAN adapter.

  @param fUserSelect
    If this parameter is set to TRUE, the functions display a dialog
    box which allows the user to select the device. If this parameter
    is set to FALSE, the function selects the first CAN controller on
    the first device found within the device list.

  @return
    VCI_OK on success, otherwise an error code
*/
//////////////////////////////////////////////////////////////////////////
HRESULT SelectDevice(BOOL fUserSelect)
{
  HRESULT hResult;

  if (fUserSelect == FALSE)
  {
    IVciDeviceManager* pDevMgr = NULL; // device manager

    _tprintf(TEXT("\n Auto Select Adapter..."));

    hResult = VciGetDeviceManager(&pDevMgr);

    if (hResult == VCI_OK)
    {
      IVciEnumDevice* pEnum = NULL; // enumerator handle
      VCIDEVICEINFO   sInfo = {0};  // device info
      VCIDEVICECAPS   sCaps = { 0 };  // device info

      hResult = pDevMgr->EnumDevices(&pEnum);

      //
      // retrieve information about the first
      // device within the device list
      //
      if (hResult == VCI_OK)
      {
        hResult = pEnum->Next(1, &sInfo, NULL);
        //
        // close the device list (no longer needed)
        //
        pEnum->Release();
        pEnum = NULL;
      }

      //
      // open the first device via device manager and get the BAL object
      //
      if (hResult == VCI_OK)
      {
        IVciDevice* pDevice = NULL;

        hResult = pDevMgr->OpenDevice(sInfo.VciObjectId, &pDevice);

        if (hResult == VCI_OK)
        {
          pDevice->GetDeviceInfo(&sInfo);
          pDevice->GetDeviceCaps(&sCaps);
          hResult = pDevice->OpenComponent(
            CLSID_VCIBAL, IID_IBalObject, (PVOID*) &pBalObj);

          pDevice->Release();
          pDevice = NULL;
        }
      }

      //
      // always select first CAN controller
      //
      lBusCtl = -1;

      if (hResult == VCI_OK)
      {
        BALFEATURES sFeatures;

        hResult = pBalObj->GetFeatures(&sFeatures);

        if (hResult == VCI_OK)
        {
          for (int i = 0; i < sFeatures.BusSocketCount; i++)
          {
            if (VCI_BUS_TYPE(sFeatures.BusSocketType[i]) == VCI_BUS_CAN)
            {
              lBusCtl = i;
              break;
            }
          }
        }
      }

      //
      // close device manager
      //
      pDevMgr->Release();
      pDevMgr = NULL;
    }


    if (hResult == VCI_OK)
      _tprintf(TEXT("OK!"));
    else
      _tprintf(TEXT("failed! Error=0x%08X"), hResult);

  }
  else
  {
    LONG lSocketNo;

    _tprintf(TEXT("\n Select Adapter by User..."));

    //
    // open a device selected by the user from ta dialog window
    //
    hResult = SocketSelectDlg(NULL,
      VCI_BUS_CAN, &pBalObj, &lSocketNo, &lBusCtl);

    DisplayError(NULL, hResult);

    if (hResult == VCI_OK)
      _tprintf(TEXT("OK!"));
    else
      _tprintf(TEXT("failed! Error=0x%08X"), hResult);
  }

  //
  // The following step is not necessary but
  // shows how to get/check BAL features.
  //
  if (hResult == VCI_OK)
    hResult = CheckFeatures();

  return( hResult );
}


//////////////////////////////////////////////////////////////////////////
/**

  Checks the features of the selected controller

  @param lCtrlNo
    number of the controller for which to to check the features

  @return
    VCI_OK on success, otherwise an error code

*/////////////////////////////////////////////////////////////////////////
HRESULT CheckFeatures(void)
{
  HRESULT hResult = VCI_E_INVALIDARG;

  if (pBalObj != NULL)
  {
    BALFEATURES sFeat;

    _tprintf(TEXT("\n Check Controller features..."));

    hResult = pBalObj->GetFeatures(&sFeat);

    if (hResult == VCI_OK)
    {
      // check if controller number is valid
      if (lBusCtl >= sFeat.BusSocketCount)
      {
        _tprintf(TEXT(" invalid controller number!"));
        hResult = VCI_E_UNEXPECTED;
      }
      else if (VCI_BUS_TYPE(sFeat.BusSocketType[lBusCtl]) != VCI_BUS_CAN)
      {
        _tprintf(TEXT(" invalid controller type!"));
        hResult = VCI_E_UNEXPECTED;
      }
      else
      {
        _tprintf(TEXT("OK!"));
      }
    }
    else
    {
      _tprintf(TEXT("failed! Error=0x%08X"), hResult);
    }
  }

  return( hResult );
}


//////////////////////////////////////////////////////////////////////////
/**

  Start CAN controller

*/////////////////////////////////////////////////////////////////////////
void StartControl(void)
{
  if (pCanCtl != NULL)
  {
    HRESULT hResult;

    //
    // initialize the CAN controller
    //

    //
    // set CAN-FD baudrate combination via raw controller settings
    // standard  1000 kBit/s
    // fast      -
    //
    CANINITLINE2 sInitParam =
    {
      CAN_OPMODE_STANDARD | 
      CAN_OPMODE_EXTENDED | CAN_OPMODE_ERRFRAME,      // opmode
      0, //CAN_EXMODE_EXTDATA  | CAN_EXMODE_FASTDATA, // exmode
      CAN_FILTER_INCL,                                // mode for line specific 11-bit ID filter
      CAN_FILTER_INCL,                                // mode for line specific 29-bit ID filter
      0,                                              // size of line specific 11-bit ID filter
      0,                                              // size of line specific 29-bit ID filter
      CAN_BTP_1000KB,                                 // standard bit rate timing
      CAN_BTP_EMPTY                                   // fast data bit rate timing
    };

    hResult = pCanCtl->InitLine(&sInitParam);

    if (hResult != VCI_OK)
    {
      _tprintf(
        TEXT("\n CAN Control InitLine failed! Error=0x%08X"),
        hResult);
    }

    //
    // set the acceptance filter
    //
    if (hResult == VCI_OK)
    { 
      hResult = pCanCtl->SetAccFilter(
        CAN_FILTER_STD, CAN_ACC_CODE_ALL, CAN_ACC_MASK_ALL);

      //
      // set the acceptance filter
      //
      if (hResult == VCI_OK)
      { 
        hResult = pCanCtl->SetAccFilter(
          CAN_FILTER_EXT, CAN_ACC_CODE_ALL, CAN_ACC_MASK_ALL);
      }

      if (hResult != VCI_OK)
      {
        _tprintf(
          TEXT("\n CAN Control SetAccFilter failed! Error=0x%08X"),
          hResult);
      }
    }

    //
    // start the CAN controller
    //
    if (hResult == VCI_OK)
    {
      hResult = pCanCtl->StartLine();
      if (hResult != VCI_OK)
      {
        _tprintf(
          TEXT("\n CAN Control StartLine failed! Error=0x%08X"),
          hResult);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////
/**
  Finalizes the application
*/
//////////////////////////////////////////////////////////////////////////
void FinalizeApp(void)
{
  //
  // release CAN control object
  //
  if (pCanCtl != NULL)
  {
    HRESULT hResult = pCanCtl->StopLine();
    if (hResult != VCI_OK)
    {
      _tprintf(
        TEXT("\n CAN Control StopLine failed! Error=0x%08X"),
        hResult);
    }

    hResult = pCanCtl->ResetLine();
    if (hResult != VCI_OK)
    {
      _tprintf(
        TEXT("\n CAN Control ResetLine failed! Error=0x%08X"),
        hResult);
    }

    pCanCtl->Release();
    pCanCtl = NULL;
  }

  //
  // release bal object
  //
  if (pBalObj != NULL)
  {
    pBalObj->Release();
    pBalObj = NULL;
  }
}


//////////////////////////////////////////////////////////////////////////
/**

  Read signals

*/////////////////////////////////////////////////////////////////////////
void ReceiveData(void)
{
  HRESULT hResult;
  UINT32  adwRxCnt[NUM_SIGNALS];

  memset(adwRxCnt, 0, sizeof(adwRxCnt));

  _tprintf(TEXT("\n\n Read Signal Set..."));

  hResult = pRxSignalSet->Read(
              TRUE,         // Specifies whether raw signal values should be
                            // converted to physical values. If set to TRUE
                            // the function converts the newly read raw signal
                            // values to physical values.
              aRxSignal,    // Pointer to an array of initialized signal
                            // buffers. The <aSignal[i].hSigId> field of
                            // each signal buffer must be initialized
                            // with the ID of the signal to be read.
              adwRxCnt,     // Pointer to an array of variables where the
                            // function stores the receive counter of the
                            // related signals. This parameter is optional
                            // and can be NULL.
              NUM_SIGNALS); // Number of signals in the array pointed to by
                            // <aSignal> and optionally <adwRxCnt>.



  if (hResult == VCI_OK)
  {
    _tprintf(TEXT("OK!\n"));

    UINT32 dwMaxCnt = 0;
    int    iWidth = 0;

    for (UINT32 i = 0; i < NUM_SIGNALS; i++)
    {
      if (dwMaxCnt < adwRxCnt[i])
        dwMaxCnt = adwRxCnt[i];
    }

    while (dwMaxCnt != 0)
    {
      iWidth++;
      dwMaxCnt /= 10;
    }

    for (UINT32 i = 0; i < NUM_SIGNALS; i++)
    {
      _tprintf(SIG_NAMES_FMT_STR, szSignalNames[i]);

      switch (aRxSignal[i].sValue.wVarType)
      {
        case FSL_VT_EMPTY: // same as VT_EMPTY
          _tprintf(TEXT("VT EMPTY  | rx-count: %*u |"), iWidth, adwRxCnt[i]);
          break;

        case FSL_VT_INT8: // same as VT_I1
          _tprintf(TEXT("VT INT8   | rx-count: %*u | "), iWidth, adwRxCnt[i]);
          _tprintf(TEXT("Value: %d"), aRxSignal[i].sValue.asInt8);
          break;

        case FSL_VT_INT16: //samer as VT_I2:
          _tprintf(TEXT("VT INT16  | rx-count: %*u | "), iWidth, adwRxCnt[i]);
          _tprintf(TEXT("Value: %d"), aRxSignal[i].sValue.asInt16);
          break;

        case FSL_VT_INT32: // same as VT_I4
          _tprintf(TEXT("VT INT32  | rx-count: %*u | "), iWidth, adwRxCnt[i]);
          _tprintf(TEXT("Value: %d"), aRxSignal[i].sValue.asInt32);
          break;

        case FSL_VT_INT64: // same as VT_I8
          _tprintf(TEXT("VT INT64  | rx-count: %*u | "), iWidth, adwRxCnt[i]);
          _tprintf(TEXT("Value: %I64d"), aRxSignal[i].sValue.asInt64);
          break;

        case FSL_VT_INT: // same as VT_INT
          _tprintf(TEXT("VT INT    | rx-count: %*u | "), iWidth, adwRxCnt[i]);
          _tprintf(TEXT("Value: %d"), aRxSignal[i].sValue.asInt);
          break;

        case FSL_VT_UINT8: // same as VT_UI1
          _tprintf(TEXT("VT UINT8  | rx-count: %*u | "), iWidth, adwRxCnt[i]);
          _tprintf(TEXT("Value: %02X"), aRxSignal[i].sValue.asUInt8);
          break;

        case FSL_VT_UINT16: //same as VT_UI2:
          _tprintf(TEXT("VT UINT16 | rx-count: %*u | "), iWidth, adwRxCnt[i]);
          _tprintf(TEXT("Value: %04X"), aRxSignal[i].sValue.asUInt16);
          break;

        case FSL_VT_UINT32: // same as VT_UI4
          _tprintf(TEXT("VT UINT32 | rx-count: %*u | "), iWidth, adwRxCnt[i]);
          _tprintf(TEXT("Value: %08X"), aRxSignal[i].sValue.asUInt32);
          break;

        case FSL_VT_UINT64: // same as VT_UI8
          _tprintf(TEXT("VT UINT64 | rx-count: %*u | "), iWidth, adwRxCnt[i]);
          _tprintf(TEXT("Value: %016I64X"), aRxSignal[i].sValue.asUInt64);
          break;

        case FSL_VT_UINT: // same as VT_UINT
          _tprintf(TEXT("VT UINT   | rx-count: %*u | "), iWidth, adwRxCnt[i]);
          _tprintf(TEXT("Value: %08X"), aRxSignal[i].sValue.asUInt);
          break;

        case FSL_VT_SINGLE: // same as VT_R4
          _tprintf(TEXT("VT SINGLE | rx-count: %*u | "), iWidth, adwRxCnt[i]);
          _tprintf(TEXT("Value: %g"), aRxSignal[i].sValue.asSingle);
          break;

        case FSL_VT_DOUBLE: // same as VT_R8
          _tprintf(TEXT("VT DOUBLE | rx-count: %*u | "), iWidth, adwRxCnt[i]);
          _tprintf(TEXT("Value: %g"), aRxSignal[i].sValue.asDouble);
          break;

        default:
          _tprintf(TEXT("VT %6d | rx-count: %*u | "),
            aRxSignal[i].sValue.wVarType, iWidth, adwRxCnt[i]);
          _tprintf(TEXT("Value: %016I64X"), aRxSignal[i].sValue.asInt64);
          break;
      }
    }
  }
  else
  {
    _tprintf(TEXT("failed! Error=0x%08X"), hResult);
  }
}


//////////////////////////////////////////////////////////////////////////
/**

  Calculation of the dew point by means of the current air temperature
  and relative humidity (in percent) using the simple Magnus formula.

  See http://en.wikipedia.org/wiki/Dew_point

  @param Temp
    current air temperature in °C

  @param RH
    current relative humidity in %

  @return
    Value of dew point in °C

*/
//////////////////////////////////////////////////////////////////////////
FLOAT DewPoint(FLOAT Temp, UINT8 RH)
{
  // Calculation of the dew point by using the Magnus parameters
  // given by Sonntag 1990 which are valid for the temperature
  // range from -45°C <= Temp <= 60°C with an error of ±0.35°C.

  // see also
  // www.sensirion.com/fileadmin/user_upload/
  //   customers/sensirion/Dokumente/2_Humidity_Sensors/
  //     Sensirion_Humidity_Sensors_at_a_Glance.pdf
  //

  //double a = 6.112;
  double b = 17.62;
  double c = 243.12;

  double Y = log((double) RH / 100) + ((b*Temp)/(c+Temp));

  return( (float) ((c*Y) / (b-Y)) );
}

//////////////////////////////////////////////////////////////////////////
/**

  Calculation of random numbers in the the half-closed interval from
  Range Min <= Random Number < Range Max.

  @param Rmin
    minimum number of the range

  @param Rmax
    minimum number of the range

  @return
    Random number in the range [Rmin, Rmax)

*/
//////////////////////////////////////////////////////////////////////////
INT8 RangeRandInt8(INT8 Rmin, INT8 Rmax)
{
  return( (INT8) (((double)rand()/(RAND_MAX+1)) * (Rmax-Rmin)) + Rmin );
}
INT16 RangeRandInt16(INT16 Rmin, INT16 Rmax)
{
  return( (INT16) (((double)rand()/(RAND_MAX+1)) * (Rmax-Rmin)) + Rmin );
}
INT32 RangeRandInt32(INT32 Rmin, INT32 Rmax)
{
  return( (INT32) (((double)rand()/(RAND_MAX+1)) * (Rmax-Rmin)) + Rmin );
}
FLOAT RangeRandFloat(FLOAT Rmin, FLOAT Rmax)
{
  return ( (FLOAT) (((double)rand()/(RAND_MAX+1)) * (Rmax-Rmin)) + Rmin );
}

//////////////////////////////////////////////////////////////////////////
/**

  Transmits signals

*/
//////////////////////////////////////////////////////////////////////////
void TransmitData(void)
{
  #ifdef TRANSMIT_SIGS

  static UINT8 AirP     = 0x00;     // hPa
  static UINT8 AirT     = 0x00;    // °C
  static UINT8 AirRH    = 0x00;       // %
  static UINT8 AirDP    = 0x00;     // °C
  static UINT8 SolRad   = 0x00;   // W/m²
  static UINT8 WindVel  = 0x00;     // km/h
  static UINT8 WindDir  = 0x00;       // N=0°, W=90°,S=180°, E=270°
  static UINT8 RainDay  = 0x00;        // mm/d
  static UINT8 RainHour = 0x00;        // mm/h

  HRESULT      hResult;

  aTxSignal[SIG_IDX_PRESSURE_AIR].sValue.asUInt8 = AirP;
  afTxDone[SIG_IDX_PRESSURE_AIR] = FALSE; // signal is currently not sent

  aTxSignal[SIG_IDX_TEMPERATURE_AIR].sValue.asUInt8 = AirT;
  afTxDone[SIG_IDX_TEMPERATURE_AIR] = FALSE; // signal is currently not sent

  aTxSignal[SIG_IDX_HUMIDITY_AIR].sValue.asUInt8 = AirRH;
  afTxDone[SIG_IDX_HUMIDITY_AIR] = FALSE; // signal is currently not sent

  aTxSignal[SIG_IDX_HUMIDITY_DEWPOINT].sValue.asUInt8 = AirDP;
  afTxDone[SIG_IDX_HUMIDITY_DEWPOINT] = FALSE; // signal is currently not sent

  aTxSignal[SIG_IDX_RADIATION_SOLAR].sValue.asUInt8 = SolRad;
  afTxDone[SIG_IDX_RADIATION_SOLAR] = FALSE; // signal is currently not sent

  aTxSignal[SIG_IDX_WIND_VELOCITY].sValue.asUInt8 = WindVel;
  afTxDone[SIG_IDX_WIND_VELOCITY] = FALSE; // signal is currently not sent

  aTxSignal[SIG_IDX_WIND_DIRECTION].sValue.asUInt8 = WindDir;
  afTxDone[SIG_IDX_WIND_DIRECTION] = FALSE; // signal is currently not sent

  aTxSignal[SIG_IDX_RAINFALL_DAILY].sValue.asUInt8 = RainDay;
  afTxDone[SIG_IDX_RAINFALL_DAILY] = FALSE; // signal is currently not sent

  aTxSignal[SIG_IDX_RAINFALL_HOURLY].sValue.asUInt8 = RainHour;
  afTxDone[SIG_IDX_RAINFALL_HOURLY] = FALSE; // signal is currently not sent
 
  //----------------------------------------------------------------------
  // Write the signals to the signal set
  //----------------------------------------------------------------------

  hResult = pTxSignalSet->Write(
    TRUE,         // Specifies whether physical values should be converted to
                  // raw signal values prior to transmission. If set to TRUE
                  // the function converts the physical values to raw signal
                  // values.
    aTxSignal,    // Pointer to an array of valid signals to write.
    afTxReady,    // Pointer to an array of initialized variables that specify
                  // which element within <aTxSignal> contains a signal ready
                  // for transmission. If <afTxReady[i]> is set to TRUE the
                  // function assumes a valid signal within <aTxSignal[i]>.
    afTxDone,     // Pointer to an array of variables where the function stores
                  // the result of the write operation. If the signal within
                  // <aTxSignal[i]> is written to the associated signal output
                  // buffer the function sets <afTxDone[i]> to TRUE, otherwise
                  // the function sets <afTxDone[i]> to FALSE. This parameter
                  // is optional and can be NULL.
    0); // Number of signal buffers in the array pointed to by
                  // <aTxSignal>, <afTxReady> and optionally <afDone>.

  if (hResult != VCI_OK)
  {
    _tprintf(TEXT("\r Write Signal Set failed! Error=0x%08X"), hResult);
  }

  #endif
}


