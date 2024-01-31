//////////////////////////////////////////////////////////////////////////
// IXXAT Automation GmbH
//////////////////////////////////////////////////////////////////////////
/**

  Declarations for the socket select dialog class.

*/
//////////////////////////////////////////////////////////////////////////
// Copyright (C) 2002-2019
// HMS Technology Center Ravensburg GmbH, all rights reserved
//////////////////////////////////////////////////////////////////////////

#ifndef SELECT_HPP
#define SELECT_HPP

//////////////////////////////////////////////////////////////////////////
//  include files
//////////////////////////////////////////////////////////////////////////
#include <vcisdk.h>

#include "dialog.hpp"

#include "VCIConsoleSample.rh"

//////////////////////////////////////////////////////////////////////////
//  constants and macros
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// data types
//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
//////////////////////////////////////////////////////////////////////////
/**
  This class implements a dialog window to select a bus socket.
*/
//////////////////////////////////////////////////////////////////////////
class CSocketSelectDlg : public CDialogWnd
{
  public:
    //---------------------------------------------------------------
    // public methods
    //---------------------------------------------------------------
    HRESULT RunModal( HWND         hWndParent,
                      UINT8        bBusType,
                      IBalObject** ppBalObject,
                      PLONG        plSocketNo,
                      PLONG        plBusCtlNo );

    //---------------------------------------------------------------
    // constructor / destructor
    //---------------------------------------------------------------
    CSocketSelectDlg();
   ~CSocketSelectDlg();

  protected:
    //---------------------------------------------------------------
    // required by CDialogWnd implementation
    //---------------------------------------------------------------
    virtual BOOL CmdProc(WORD wId, WORD wCode, HWND hCtrl);
    virtual BOOL DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

  private:
    //---------------------------------------------------------------
    // Handler for dialog messages
    //---------------------------------------------------------------
    BOOL OnWmInitdialog(WPARAM wParam, LPARAM lParam);
    BOOL OnWmDestroy   (WPARAM wParam, LPARAM lParam);
    BOOL OnWmUpdate    (WPARAM wParam, LPARAM lParam);

    //---------------------------------------------------------------
    // Handler for control messages
    //---------------------------------------------------------------
    BOOL OnBtnOk     (WORD wCode, HWND hCtrl);
    BOOL OnBtnCancel (WORD wCode, HWND hCtrl);
    BOOL OnLbxDevList(WORD wCode, HWND hCtrl);
    BOOL OnCbxSocList(WORD wCode, HWND hCtrl);

    //---------------------------------------------------------------
    // device list support functions
    //---------------------------------------------------------------
    void DeviceListClear ( void );
    void DeviceListUpdate( void );
    void DeviceListSelect( void );

    LRESULT DeviceListSearch( VCIID vciid );
    LRESULT DeviceListInsert( VCIID vciid );

    //---------------------------------------------------------------
    // socket list support functions
    //---------------------------------------------------------------
    void SocketListUpdate( void );
    void SocketListSelect( void );

    //---------------------------------------------------------------
    // utility functions
    //---------------------------------------------------------------
    void UpdateView( IVciDevice* pDevice );

    //---------------------------------------------------------------
    // device list monitor functions
    //---------------------------------------------------------------
    HRESULT ThreadStart( void );
    HRESULT ThreadStop ( BOOL fWaitFor );

    static DWORD WINAPI ThreadProc( CSocketSelectDlg* pDialog );

    //---------------------------------------------------------------
    // data members
    //---------------------------------------------------------------
    volatile LONG      m_lTerminate;    // flag to exit thread
    HANDLE             m_hThread;       // handle to change monitor thread
    DWORD              m_dwThdId;       // id to change monitor thread

    HANDLE             m_hChgEvt;       // handle to change event
    IVciDeviceManager* m_pDevMan;       // points to the device manager
    IVciEnumDevice*    m_pDevEnu;       // points to the device enumerator
    IBalObject*        m_pBalObj;       // points to the selected BAL object
    UINT8              m_BusType;       // type of the listed bus sockets
    LONG               m_lCurDev;       // index of the currently selected device
    LONG               m_lSocket;       // number of the selected bus socket
    LONG               m_lCtrlNo;       // number of the selected bus controller
};

void DisplayError(HWND hParent, HRESULT hResult);

#endif // __cplusplus


EXTERN_C HRESULT SocketSelectDlg( HWND          hWndParent,
                                  UINT8         bBusType,
                                  IBalObject**  ppBalObject,
                                  PLONG         plSocketNo,
                                  PLONG         plBusCtlNo );

#endif //_SOCDLG_HPP_
