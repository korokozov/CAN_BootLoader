//////////////////////////////////////////////////////////////////////////
// IXXAT Automation GmbH
//////////////////////////////////////////////////////////////////////////
/**

  Implementation of the socket select dialog class.

*/
//////////////////////////////////////////////////////////////////////////
// Copyright (C) 2002-2019
// HMS Technology Center Ravensburg GmbH, all rights reserved
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "SocketSelectDlg.hpp"

//////////////////////////////////////////////////////////////////////////
// static constants, types, macros, variables
//////////////////////////////////////////////////////////////////////////
#define WM_UPDATE  (WM_USER+1)    // update message

//////////////////////////////////////////////////////////////////////////
// static functions
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
  This function displays a message box for the specified error code.

  @param hResult
    Error code or -1 to display the error code returned by GetLastError().
*/
//////////////////////////////////////////////////////////////////////////
void DisplayError(HWND hParent, HRESULT hResult)
{
  TCHAR szError[1024];

  if (hResult != NO_ERROR)
  {
    if (hResult == -1)
      hResult = GetLastError();

    szError[0] = 0;
    VciFormatError(hResult, szError, NUMELEM(szError));

    // VciFormatErrorEx(hResult, szError, sizeof(szError)-1);
    MessageBox(hParent, szError, TEXT("VCI Demo"), MB_OK | MB_ICONSTOP);
  }
}

//////////////////////////////////////////////////////////////////////////
// message handlers for CSocketSelectDlg
//////////////////////////////////////////////////////////////////////////
BEGIN_DIALOG_PROC(CSocketSelectDlg)
 ON_MESSAGE( WM_INITDIALOG, OnWmInitdialog )
 ON_MESSAGE( WM_DESTROY,    OnWmDestroy    )
 ON_MESSAGE( WM_UPDATE,     OnWmUpdate     )
END_DIALOG_PROC(CDialogWnd)

//////////////////////////////////////////////////////////////////////////
// command handler for CCanSelectDlg
//////////////////////////////////////////////////////////////////////////
BEGIN_COMMAND_PROC(CSocketSelectDlg)
 ON_COMMAND( IDOK,        OnBtnOk      )
 ON_COMMAND( IDCANCEL,    OnBtnCancel  )
 ON_COMMAND( IDC_DEVLIST, OnLbxDevList )
 ON_COMMAND( IDC_SOCLIST, OnCbxSocList )
END_COMMAND_PROC(CDialogWnd)

//////////////////////////////////////////////////////////////////////////
/**
  Constructor for the socket select dialog window class.
*/
//////////////////////////////////////////////////////////////////////////
CSocketSelectDlg::CSocketSelectDlg()
{
  m_lTerminate = 0;
  m_hThread = NULL;
  m_dwThdId = 0;

  m_hChgEvt = NULL;
  m_pDevMan = NULL;
  m_pDevEnu = NULL;
  m_pBalObj = NULL;
  m_lCurDev = -2;
  m_lSocket = -1;
  m_lCtrlNo = -1;
}

//////////////////////////////////////////////////////////////////////////
/**
  Destructor for the socket select dialog window class.
*/
//////////////////////////////////////////////////////////////////////////
CSocketSelectDlg::~CSocketSelectDlg()
{
  ThreadStop(TRUE);

  m_lSocket = -1;
  m_lCtrlNo = -1;

  if (m_pBalObj != NULL)
  {
    m_pBalObj->Release();
    m_pBalObj = NULL;
  }

  if (m_pDevMan)
  {
    m_pDevMan->Release();
    m_pDevMan = NULL;
  }
}

//////////////////////////////////////////////////////////////////////////
/**
  The function serves as the starting address for the monitor thread.
  The functions address is specified when creating the monitor thread.

  @param pDialog
    Pointer to the dialog object.

  @return
    alway S_OK ( 0 )
*/
//////////////////////////////////////////////////////////////////////////
DWORD CSocketSelectDlg::ThreadProc(CSocketSelectDlg* pDialog)
{
  while (!pDialog->m_lTerminate)
  {
    // We cannot send a message if the thread is terminating,
    // because the ThreadStop call within OnDestroy is waiting
    // for the thread to terminate and does not respond to the
    // message queue.
    pDialog->SendMessage(WM_UPDATE, 0, 0);

    WaitForSingleObject(pDialog->m_hChgEvt, INFINITE);
  }

  return( 0 );
}

//////////////////////////////////////////////////////////////////////////
/**
  This function starts execution of the device list monitor thread.

  @return
    The function returns NO_ERROR on success, otherwise an error value.
*/
//////////////////////////////////////////////////////////////////////////
HRESULT CSocketSelectDlg::ThreadStart( )
{
  // result is ok if thread already created
  HRESULT hResult = VCI_OK;

  if (m_hThread == NULL)
  {
    m_lTerminate = 0;

    m_hThread = CreateThread(NULL, 0, (PTHREAD_START_ROUTINE) ThreadProc,
                             (LPVOID) this, CREATE_SUSPENDED, &m_dwThdId);

    if (m_hThread != NULL)
    {
      ResumeThread(m_hThread);
    }
    else
    {
      hResult = GetLastError();
    }
  }

  return(hResult);
}

//////////////////////////////////////////////////////////////////////////
/**
  This function stops execution of the device list monitor thread.

  @param fWaitFor
    If this parameter is set to TRUE the functions waits until the execution of 
    the thread ends.

  @return
    The function returns NO_ERROR on success, otherwise an error value.
*/
//////////////////////////////////////////////////////////////////////////
HRESULT CSocketSelectDlg::ThreadStop(BOOL fWaitFor)
{
  HRESULT hResult = NO_ERROR;

  if (m_hThread != NULL)
  {
    if (GetCurrentThreadId() != m_dwThdId)
    {
      //
      // signal thread termination
      //
      InterlockedIncrement(&m_lTerminate);
      SetEvent(m_hChgEvt);

      //
      // wait for termination
      //
      if (fWaitFor == TRUE)
      {
        while(ResumeThread(m_hThread) > 1);

        switch( WaitForSingleObject(m_hThread, INFINITE) )
        {
          case WAIT_OBJECT_0 : hResult = NO_ERROR;          break;
          case WAIT_FAILED   : hResult = GetLastError();    break;
          default            : hResult = ERROR_GEN_FAILURE; break;
        }

        CloseHandle(m_hThread);
        m_hThread = NULL;
        m_dwThdId = 0;
      }
    }
    else
    {
      ExitThread(NO_ERROR); // this function never returns
    }
  }

  return( hResult );
}

//////////////////////////////////////////////////////////////////////////
/**
  This function updates the view of the dialog.

  @param hDevice
    Handle to the VCI Device of the currently selected device. This parameter is 
    set to NULL if no device is selected.
*/
//////////////////////////////////////////////////////////////////////////
void CSocketSelectDlg::UpdateView(IVciDevice* pDevice)
{
  VCIDEVICEINFO sInfo;
  TCHAR         szGuid[256];
  TCHAR         szText[256];

  //
  // update device info view
  //
  if (pDevice &&
      (VCI_OK == pDevice->GetDeviceInfo(&sInfo)))
  {
    SetItemTextA(IDC_DESCRIPTION,  sInfo.Description);
    SetItemTextA(IDC_MANUFACTURER, sInfo.Manufacturer);

    wsprintf(szText, TEXT("%d.%02d"),
             sInfo.DriverMajorVersion,
             sInfo.DriverMinorVersion);
    SetItemText(IDC_DRIVERVERSION, szText);

    wsprintf(szText, TEXT("%d.%02d"),
             sInfo.HardwareMajorVersion,
             sInfo.HardwareMinorVersion);
    SetItemText(IDC_HARDWAREVERSION, szText);

    VciGuidToChar(sInfo.DeviceClass, szGuid, NUMELEM(szGuid));
    SetItemText(IDC_DEVICECLASS, szGuid);

    VciGuidToChar(sInfo.UniqueHardwareId.AsGuid, szGuid, NUMELEM(szGuid));
    SetItemText(IDC_HARDWAREID, szGuid);

    VciLuidToChar(sInfo.VciObjectId, szGuid, NUMELEM(szGuid));
    SetItemText(IDC_VCIOBJECTID, szGuid);
  }
  else
  {
    SetItemText(IDC_DESCRIPTION,     TEXT(""));
    SetItemText(IDC_MANUFACTURER,    TEXT(""));
    SetItemText(IDC_DRIVERVERSION,   TEXT(""));
    SetItemText(IDC_HARDWAREVERSION, TEXT(""));
    SetItemText(IDC_DEVICECLASS,     TEXT(""));
    SetItemText(IDC_HARDWAREID,      TEXT(""));
    SetItemText(IDC_VCIOBJECTID,     TEXT(""));
  }
}

//////////////////////////////////////////////////////////////////////////
/**
  This function adds the specified device to the device list.

  @param vciid
    VCIID of the device to add

  @return
    The return value is the zero-based index of the added device. If an error 
    occurs, the return value is LB_ERR. If there is insufficient space to store 
    the new device, the return value is LB_ERRSPACE. 
*/
//////////////////////////////////////////////////////////////////////////
LRESULT CSocketSelectDlg::DeviceListInsert(VCIID vciid)
{
  IVciDevice*   pDevice = NULL;
  LRESULT       lIndex;
  TCHAR         szText[512];
  VCIDEVICEINFO sInfo;
  VCIDEVICECAPS sCaps;

  lIndex = DeviceListSearch(vciid);

  if ((lIndex == LB_ERR) && (m_pDevMan != NULL))
  {
    if (m_pDevMan->OpenDevice(vciid, &pDevice) == VCI_OK)
    {
      if ((pDevice->GetDeviceInfo(&sInfo) == VCI_OK) &&
          (pDevice->GetDeviceCaps(&sCaps) == VCI_OK) &&
          (sCaps.BusCtrlCount > 0))
      {
#ifdef UNICODE
        wsprintf(szText, TEXT("%08X%08X - %S"),
          vciid.AsLuid.HighPart, vciid.AsLuid.LowPart, sInfo.Description);
#else
        wsprintf(szText, TEXT("%08X%08X - %s"),
          vciid.AsLuid.HighPart, vciid.AsLuid.LowPart, sInfo.Description);
#endif

        lIndex = SendItemMessage(IDC_DEVLIST,
                   LB_ADDSTRING, 0, (LPARAM) szText);

        if (lIndex >= 0)
        {
          pDevice->AddRef();
          SendItemMessage(IDC_DEVLIST,
            LB_SETITEMDATA, lIndex, (LPARAM) pDevice);
        }
      }

      pDevice->Release();
    }
  }

  return( lIndex );
}

//////////////////////////////////////////////////////////////////////////
/**
  This function searches for the specified device within the device list.

  @param vciid
    VCIID of the device to search for.

  @return
    The return value is the zero-based index of the matching item, or LB_ERR if 
    the search was unsuccessful. 
*/
//////////////////////////////////////////////////////////////////////////
LRESULT CSocketSelectDlg::DeviceListSearch(VCIID vciid)
{
  LRESULT       lResult;
  LRESULT       lCount;
  LRESULT       lIndex;
  LRESULT       lData;
  VCIDEVICEINFO sInfo;

  lResult = LB_ERR;

  lCount = SendItemMessage(IDC_DEVLIST, LB_GETCOUNT, 0, 0);

  if (lCount != LB_ERR)
  {
    for (lIndex = 0; lIndex < lCount; lIndex++)
    {
      lData = SendItemMessage(IDC_DEVLIST, LB_GETITEMDATA, lIndex, 0);

      if ((lData != 0) && (lData != LB_ERR))
      {
        IVciDevice* pDevice = (IVciDevice*)lData;

        if ((pDevice->GetDeviceInfo(&sInfo) == VCI_OK) &&
            (sInfo.VciObjectId.AsInt64 == vciid.AsInt64))
        {
          lResult = lIndex;
          break;
        }
      }
    }
  }

  return( lResult );
}

//////////////////////////////////////////////////////////////////////////
/**
  This function clears the device list.
*/
//////////////////////////////////////////////////////////////////////////
void CSocketSelectDlg::DeviceListClear( )
{
  LRESULT lCount;
  LRESULT lData;

  lCount = SendItemMessage(IDC_DEVLIST, LB_GETCOUNT, 0, 0);

  if (lCount != LB_ERR)
  {
    for (LRESULT lIndex = 0; lIndex < lCount; lIndex++)
    {
      lData = SendItemMessage(IDC_DEVLIST, LB_GETITEMDATA, lIndex, 0);

      if ((lData != 0) && (lData != LB_ERR))
      {
        // release device object
        IVciDevice* pDevice = (IVciDevice*)lData;
        pDevice->Release();
        SendItemMessage(IDC_DEVLIST, LB_SETITEMDATA, lIndex, 0);
      }
    }
  }

  SendItemMessage(IDC_DEVLIST, LB_RESETCONTENT, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
/**
  This function updates the device list.
*/
//////////////////////////////////////////////////////////////////////////
void CSocketSelectDlg::DeviceListUpdate( )
{
  LRESULT       lCount;
  LRESULT       lData;
  VCIDEVICEINFO sInfo;

  //
  // remove all unavailable devices from the device list
  //
  lCount = SendItemMessage(IDC_DEVLIST, LB_GETCOUNT, 0, 0);

  for (LRESULT lIndex = 0; lIndex < lCount; lIndex++)
  {
    lData = SendItemMessage(IDC_DEVLIST, LB_GETITEMDATA, lIndex, 0);

    IVciDevice* pDevice = (IVciDevice*)lData;

    if ((lData   != LB_ERR) && 
        (pDevice != NULL))
    {
      if (pDevice->GetDeviceInfo(&sInfo) != VCI_OK)
      {
        pDevice->Release();
        lData = 0;
      }

      if (lData == 0)
        SendItemMessage(IDC_DEVLIST, LB_DELETESTRING, lIndex, 0);
    }
  }

  //
  // The selection may change because the currently selected
  // item is deleted.
  //
  DeviceListSelect();

  //
  // enumerate all devices
  //
  if (m_pDevEnu != NULL)
  {
    m_pDevEnu->Reset();

    while (m_pDevEnu->Next(1, &sInfo, NULL) == VCI_OK)
      DeviceListInsert(sInfo.VciObjectId);
  }
}
//////////////////////////////////////////////////////////////////////////
/**
  This function is called whenever a device is selected within the device list 
  combo box.
*/
//////////////////////////////////////////////////////////////////////////
void CSocketSelectDlg::DeviceListSelect( )
{
  LRESULT lIndex;
  LRESULT lData;
  IVciDevice* pDevice = NULL;

  lIndex = SendItemMessage(IDC_DEVLIST, LB_GETCURSEL, 0, 0);

  if (lIndex >= 0)
  {
    lData = SendItemMessage(IDC_DEVLIST, LB_GETITEMDATA, lIndex, 0);
    if ((lData != 0) && (lData != LB_ERR))
      pDevice = (IVciDevice*)lData;
  }

  if (lIndex != m_lCurDev)
  {
    //
    // release currently selected BAL
    //
    if (m_pBalObj != NULL)
    {
      m_pBalObj->Release();
      m_pBalObj = NULL;
    }

    //
    // open BAL of newly selected device
    //
    if (pDevice != NULL)
    {
      HRESULT hResult = pDevice->OpenComponent(CLSID_VCIBAL,
                          IID_IBalObject, (PVOID*) &m_pBalObj);
      DisplayError(CDialogWnd::GetHandle(), hResult);
    }

    //
    // update dialog view
    //
    UpdateView(pDevice);
    SocketListUpdate();
    m_lCurDev = (LONG) lIndex;
  }
}

//////////////////////////////////////////////////////////////////////////
/**
  This function updates the bus socket list.
*/
//////////////////////////////////////////////////////////////////////////
void CSocketSelectDlg::SocketListUpdate( )
{
  BALFEATURES Features;
  TCHAR         szText[32];

  SendItemMessage(IDC_SOCLIST, CB_RESETCONTENT, 0, 0);
  EnableControl(IDC_SOCLIST, FALSE);

  m_lCtrlNo = -1;
  m_lSocket = -1;

  if ((m_pBalObj != NULL) && (m_pBalObj->GetFeatures(&Features) == VCI_OK))
  {
    char cNumSocs = 0;

    for (UINT8 s = 0; s < Features.BusSocketCount; s++)
    {
      if (VCI_BUS_TYPE(Features.BusSocketType[s]) == m_BusType)
      {
        switch (m_BusType)
        {
          case VCI_BUS_CAN:
            wsprintf(szText, TEXT("CAN - %i"), cNumSocs + 1);
            break;
          case VCI_BUS_LIN:
            wsprintf(szText, TEXT("LIN - %i"), cNumSocs + 1);
            break;
        }

        cNumSocs++;

        LONG lIndex = (LONG) SendItemMessage(IDC_SOCLIST, CB_ADDSTRING, 0, (LPARAM) szText);
        
        if (lIndex >= 0) // store socket number within item data
          SendItemMessage(IDC_SOCLIST, CB_SETITEMDATA, lIndex, s);
      }
    }

    EnableControl(IDC_SOCLIST, cNumSocs > 0);
  }
}

//////////////////////////////////////////////////////////////////////////
/**

  This function is called whenever a CAN socket is selected within the
  CAN socket list combo box.

*/
//////////////////////////////////////////////////////////////////////////
void CSocketSelectDlg::SocketListSelect( )
{
  LRESULT lIndex;

  //
  // retrieve currently selected socket
  //
  lIndex = SendItemMessage(IDC_SOCLIST, CB_GETCURSEL, 0, 0);

  if (lIndex >= 0)
  {
    m_lCtrlNo = (LONG) lIndex;
    m_lSocket = (LONG) SendItemMessage(IDC_SOCLIST, CB_GETITEMDATA, lIndex, 0);
  }
  else
  {
    m_lCtrlNo = -1;
    m_lSocket = -1;
  }
}

//////////////////////////////////////////////////////////////////////////
/**
  This function is called when the window receives the WM_INITDIALOG message.

  @param wParam
    Handle to the control to receive the default keyboard focus. The system 
    assigns the default keyboard focus only if the dialog box procedure returns 
    TRUE. 
  @param lParam
    Specifies additional initialization data. This data is passed to the system 
    as the lParam parameter in a call to the Create function used to create the 
    dialog box.

  @return
    The dialog box procedure should return TRUE to direct the system to set the 
    keyboard focus to the control given by hwndFocus. Otherwise, it should 
    return FALSE to prevent the system from setting the default keyboard focus. 
    The dialog box procedure should return the value directly. The DWL_MSGRESULT
    value set by the SetWindowLong function is ignored. 
*/
//////////////////////////////////////////////////////////////////////////
BOOL CSocketSelectDlg::OnWmInitdialog(WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(wParam);
  UNREFERENCED_PARAMETER(lParam);

  HRESULT hResult;

  //
  // release currently selected device
  //
  if (m_pBalObj != NULL)
  {
    m_pBalObj->Release();
    m_pBalObj = NULL;
  }

  m_lCurDev = -2;
  m_lSocket = -1;
  m_lCtrlNo = -1;

  UpdateView(NULL);
  DeviceListClear();

  //
  // get VCI device manager
  //
  hResult = VciGetDeviceManager(&m_pDevMan);

  //
  // get VCI device enumerator
  //
  if (hResult == VCI_OK)
    hResult = m_pDevMan->EnumDevices(&m_pDevEnu);

  //
  // initialize the device list and a thread that waits on
  // the change event from the device enumerator
  //
  if (hResult == VCI_OK)
  {
    //
    // create change event and assign it to the enumerator
    //
    m_hChgEvt = CreateEvent(NULL, 0, 0, NULL);

    if (m_hChgEvt != NULL)
      hResult = m_pDevEnu->AssignEvent(m_hChgEvt);
    else
      hResult = GetLastError();

    //
    // start update thread
    //
    if (hResult == VCI_OK)
      hResult = ThreadStart();

    //
    // set change event to simulate a device list change
    //
    // if (hResult == VCI_OK)
    //   SetEvent(m_hChgEvt);
  }

  DisplayError(CDialogWnd::GetHandle(), hResult);

  return(TRUE);
}

//////////////////////////////////////////////////////////////////////////
/**
  This function is called when the window receives the WM_DESTROY message.

  @param wParam
    not used
  @param lParam
    not used

  @return
    This function should always return FALSE.
*/
//////////////////////////////////////////////////////////////////////////
BOOL CSocketSelectDlg::OnWmDestroy(WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(wParam);
  UNREFERENCED_PARAMETER(lParam);
  //
  // terminate update thread
  //
  ThreadStop(TRUE);

  //
  // close change event
  //
  if (m_hChgEvt)
  {
    CloseHandle(m_hChgEvt);
    m_hChgEvt = NULL;
  }

  //
  // release VCI device enumerator and device manager
  //
  if (m_pDevEnu != NULL)
  {
    m_pDevEnu->Release();
    m_pDevEnu = NULL;
  }

  if (m_pDevMan != NULL)
  {
    m_pDevMan->Release();
    m_pDevMan = NULL;
  }

  //
  // clear device list
  //
  DeviceListClear();
  UpdateView(NULL);

  return(FALSE);
}

//////////////////////////////////////////////////////////////////////////
/**
  This function is called when the window receives the WM_UPDATE message.

  @param wParam
    not used
  @param lParam
    not used

  @return
    This function should always return TRUE.
*/
//////////////////////////////////////////////////////////////////////////
BOOL CSocketSelectDlg::OnWmUpdate(WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(wParam);
  UNREFERENCED_PARAMETER(lParam);

  DeviceListUpdate();
  return(TRUE);
}

//////////////////////////////////////////////////////////////////////////
/**
  This function is called for notifications from the device list.

  @param wCode
    Action code
  @param hCtrl
    Handle of control

  @return
    This function should always return TRUE.
*/
//////////////////////////////////////////////////////////////////////////
BOOL CSocketSelectDlg::OnLbxDevList(WORD wCode, HWND hCtrl)
{
  UNREFERENCED_PARAMETER(hCtrl);

  switch (wCode)
  {
    case LBN_SELCHANGE :
      DeviceListSelect();
      break;
  }

  return(TRUE);
}

//////////////////////////////////////////////////////////////////////////
/**
  This function is called for notifications from the bus socket list.

  @param wCode
    Action code
  @param hCtrl
    Handle of control

  @return
    This function should always return TRUE.
*/
//////////////////////////////////////////////////////////////////////////
BOOL CSocketSelectDlg::OnCbxSocList(WORD wCode, HWND hCtrl)
{
  UNREFERENCED_PARAMETER(hCtrl);

  switch (wCode)
  {
    case CBN_SELCHANGE :
      SocketListSelect();
      break;
  }

  return(TRUE);
}

//////////////////////////////////////////////////////////////////////////
/**
  This function is called whenever the "OK" button is pressed.

  @param wCode
    Action code
  @param hCtrl
    Handle of control

  @return
    This function should always return TRUE.
*/
//////////////////////////////////////////////////////////////////////////
BOOL CSocketSelectDlg::OnBtnOk(WORD wCode, HWND hCtrl)
{
  UNREFERENCED_PARAMETER(wCode);
  UNREFERENCED_PARAMETER(hCtrl);

  if (m_pBalObj != NULL)
  {
    //
    // test if the selected bus socket is correct
    //
    if (m_lSocket >= 0)
    {
      GUID      sIid;
      IUnknown* pSoc;
      switch (m_BusType)
      {
        case VCI_BUS_CAN: sIid = IID_ICanSocket; break;
        case VCI_BUS_LIN: sIid = IID_ILinSocket; break;
        default         : sIid = GUID_NULL;      break;
      }

      if (m_pBalObj->OpenSocket(m_lSocket, sIid, (PVOID*) &pSoc) == VCI_OK)
        pSoc->Release();
      else
        m_lSocket = -1;
    }

    if (m_lSocket >= 0)
    {
      EndDialog(IDOK);
    }
    else
    {
      TCHAR szTitle[256];
      GetWindowText(szTitle, 256);
      MessageBox(CDialogWnd::GetHandle(),
        TEXT("Please select a BUS line !"), szTitle, MB_OK | MB_ICONSTOP);
    }
  }
  else
  {
    TCHAR szTitle[256];
    GetWindowText(szTitle, 256);
    MessageBox(CDialogWnd::GetHandle(),
      TEXT("Please select a device !"), szTitle, MB_OK | MB_ICONSTOP);
  }

  return(TRUE);
}

//////////////////////////////////////////////////////////////////////////
/**
  This function is called whenever the "Cancel" button is pressed.

  @param wCode
    Action code
  @param hCtrl
    Handle of control

  @return
    This function should always return TRUE.
*/
//////////////////////////////////////////////////////////////////////////
BOOL CSocketSelectDlg::OnBtnCancel(WORD wCode, HWND hCtrl)
{
  UNREFERENCED_PARAMETER(wCode);
  UNREFERENCED_PARAMETER(hCtrl);

  EndDialog(IDCANCEL);
  return(TRUE);
}

//////////////////////////////////////////////////////////////////////////
/**
  This function runs a modal dialog box to select a bus controller.

  @param hWndParent
    Handle to the window that owns the dialog box.
  @param bBusType
    Type of the bus socket to select. This can be one of the VCI_BUS_ constants.
  @param phDevice
    Pointer to a variable where the function stores the handle of the selected 
    device.
  @param plCtrlNo
    Pointer to a variable where the function stores the number of the selected 
    bus controller.

  @return
    If the function succeeds it returns VCI_OK, otherwise an error code other 
    than VCI_OK. The function returns VCI_E_ABORT if the user cancels the 
    dialog box.

  @note
    The caller is responsible to close the handle returned in <phDevice>.
*/
//////////////////////////////////////////////////////////////////////////
HRESULT CSocketSelectDlg::RunModal(HWND         hWndParent,
                                   UINT8        bBusType,
                                   IBalObject** ppBalObject,
                                   PLONG        plSocketNo,
                                   PLONG        plBusCtlNo)

{
  INT_PTR iResult;
  HRESULT hResult;

  if ((ppBalObject != NULL) &&
      (plSocketNo != NULL)  &&
      ((bBusType == VCI_BUS_CAN) ||
       (bBusType == VCI_BUS_LIN) ||
       (bBusType == VCI_BUS_FXR)) )
  {
    *ppBalObject = NULL;
    *plSocketNo  = -1;

    if (plBusCtlNo != NULL)
      *plBusCtlNo = -1;

    m_BusType = bBusType;

    iResult = DialogBox(GetModuleHandle(NULL),
                        TEXT("SOCSELECTDLG"), hWndParent);

    if (iResult == IDOK)
    {
      if ((m_pBalObj != NULL) && (m_lSocket >= 0))
      {
        m_pBalObj->AddRef();

        *ppBalObject = m_pBalObj;
        *plSocketNo  = m_lSocket;

        if (plBusCtlNo != NULL)
          *plBusCtlNo = m_lCtrlNo;

        hResult = VCI_OK;
      }
      else
      {
        hResult = VCI_E_FAIL; // this should never happen
      }
    }
    else
    {
      if (iResult == IDCANCEL)
        hResult = VCI_E_ABORT;
      else
        hResult = GetLastError();
    }
  }
  else
  {
    hResult = VCI_E_INVALIDARG;
  }

  return( hResult );
}

//////////////////////////////////////////////////////////////////////////
/**
  This function runs a modal dialog box to select a bus controller.

  @param hWndParent
    Handle to the window that owns the dialog box.
  @param bBusType
    Type of the bus socket to select. This can be one of the VCI_BUS_ constants.
  @param ppBalObject
    Pointer to a variable where the function stores a pointer to the BAL opbject
    of the selected device.
  @param plSocketNo
    Pointer to a variable where the function stores the number the number of
    the selected bus socket.
  @param plBusCtlNo
    Pointer to a variable where the function stores the number of the selected
    bus controller.

  @return
    If the function succeeds it returns VCI_OK, otherwise an error code other
    than VCI_OK. The function returns VCI_E_ABORT if the user cancels the
    dialog box.

  @note
    The caller is responsible to close the handle returned in <phDevice>.
*/
//////////////////////////////////////////////////////////////////////////
EXTERN_C HRESULT SocketSelectDlg(HWND         hWndParent,
                                 UINT8        bBusType,
                                 IBalObject** ppBalObject,
                                 PLONG        plSocketNo,
                                 PLONG        plBusCtlNo)
{
  static BOOL fIsInit = FALSE;
  HRESULT     hResult = NO_ERROR;

  if (!fIsInit)
  {
    WNDCLASS wc;

    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = TEXT("CDialogWnd");

    fIsInit = CDialogWnd::RegisterClass(&wc);

    if (!fIsInit)
      hResult = GetLastError();
  }

  if (hResult == NO_ERROR)
  {
    CSocketSelectDlg oDialog;
    hResult = oDialog.RunModal(hWndParent, bBusType, ppBalObject, plSocketNo, plBusCtlNo);
  }

  return( hResult );
}


#ifdef MICROSOFT_C
#pragma warning(default:4100) // unreferenced formal parameter
#endif
