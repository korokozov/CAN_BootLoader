//////////////////////////////////////////////////////////////////////////
// HMS Technology Center Ravensburg GmbH
//////////////////////////////////////////////////////////////////////////
/**

 Declarations for Windows dialog classes.

*/
//////////////////////////////////////////////////////////////////////////
// Copyright (C) 2001-2019
// HMS Technology Center Ravensburg GmbH, all rights reserved
//////////////////////////////////////////////////////////////////////////

#ifndef _DIALOG_HPP_
#define _DIALOG_HPP_

//////////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <windowsx.h>
#include <stdtype.h>

#ifdef __cplusplus

//////////////////////////////////////////////////////////////////////////
// static constants, types, macros, variables
//////////////////////////////////////////////////////////////////////////

#define DLGCBPARAM  WPARAM wParam, LPARAM lParam
#define CMDCBPARAM  WORD wCode, HWND hCtrl

//------------------------------------------------------------------------
// Macro:
//  ON_MESSAGE
//
// Description:
//  This macros expands to a case statement for the switch case declared
//  by the BEGIN_DIALOG_PROG macro.
//
// Arguments:
//  Number -> Number of the message (constant)
//  Method -> method to be called
//------------------------------------------------------------------------
#define ON_MESSAGE(Number, Method) \
        case (Number): return( Method##(wp, lp) );

//------------------------------------------------------------------------
// Macro:
//  ON_COMMAND
//
// Description:
//  This macros expands to a case statement for the switch case declared
//  by the BEGIN_COMMAND_PROC macro.
//
// Arguments:
//  CtrlID -> ID of the control
//  Method -> method to be called
//------------------------------------------------------------------------
#define ON_COMMAND(CtrlID, Method) \
        case (CtrlID): return( Method##(wCode, hCtrl) );

//------------------------------------------------------------------------
// Macro:
//  BEGIN_DIALOG_PROC
//
// Description:
//  This macros can be used to declare a message handler for a dialog box.
//  It expands to the ClassName::DialogProc function which implements a
//  switch case for message handling.
//
// Arguments:
//  Class -> Name of the dialog class.
//------------------------------------------------------------------------
#define BEGIN_DIALOG_PROC(Class) \
        BOOL Class::DlgProc(UINT msg, WPARAM wp, LPARAM lp) \
        { switch (msg) {

//------------------------------------------------------------------------
// Macro:
//  END_DIALOG_PROC
//
// Description:
//  This macros ends the message handler declared by the BEGIN_DIALOG_PROG
//  macro. The macro expands to code which closes the switch case and call
//  the DialogProc method of the specified base class for all messages not
//  handled by a DLGEVENT macro.
//
// Arguments:
//  Base -> Base class of the dialog (usually CDialogWnd).
//------------------------------------------------------------------------
#define END_DIALOG_PROC(Base) \
        } return( Base::DlgProc(msg, wp, lp) ); }

//------------------------------------------------------------------------
// Macro:
//  BEGIN_COMMAND_PROC
//
// Description:
//  This macros can be used to declare a command message handler for a
//  dialog box. It expands to the ClassName::CommandProc function which
//  implements a switch case for WM_COMMAND message handling.
//
// Arguments:
//  Class -> Name of the dialog class.
//------------------------------------------------------------------------
#define BEGIN_COMMAND_PROC(Class) \
        BOOL Class::CmdProc(WORD wId, WORD wCode, HWND hCtrl) \
        { switch (wId) {

//------------------------------------------------------------------------
// Macro:
//  END_COMMAND_PROC
//
// Description:
//  This macros ends the message handler declared by the BEGIN_COMMAND_PROC
//  macro. The macro expands to code which close the switch case and call
//  the CommandProc method of the specified base class for all commands not
//  handled by a COMMAND macro.
//
// Arguments:
//  Base -> Base class of the dialog (usually CDialogWnd).
//------------------------------------------------------------------------
#define END_COMMAND_PROC(Base) \
        } return( Base::CmdProc(wId, wCode, hCtrl) ); }


/*****************************************************************************
 Class:
  CDialogWnd

 Description:
  This class implements the standard dialog class.
*****************************************************************************/
class CDialogWnd
{
  public:
    static BOOL RegisterClassA    ( PWNDCLASSA pWndClass );
    static BOOL RegisterClassW    ( PWNDCLASSW pWndClass );
    static BOOL RegisterClassExA  ( PWNDCLASSEXA pWndClsEx );
    static BOOL RegisterClassExW  ( PWNDCLASSEXW pWndClsEx );

    BOOL     CreateDialogParamA   ( HINSTANCE hInstance, PCSTR pTemplateName,
                                    HWND hWndParent, LPARAM lInitParam = 0 );
    BOOL     CreateDialogParamW   ( HINSTANCE hInstance, PCWSTR pTemplateName,
                                    HWND hWndParent, LPARAM lInitParam = 0 );
    INT_PTR  DialogBoxParamA      ( HINSTANCE hInstance, PCSTR pTemplateName,
                                    HWND hWndParent, LPARAM lInitParam = 0 );
    INT_PTR  DialogBoxParamW      ( HINSTANCE hInstance, PCWSTR pTemplateName,
                                    HWND hWndParent, LPARAM lInitParam = 0 );
    BOOL     EndDialog            ( int nResult = 0 );
    BOOL     ShowWindow           ( int nCmdShow = SW_SHOWDEFAULT );
    BOOL     UpdateWindow         ( void );
    BOOL     EnableWindow         ( BOOL fEnable );
    BOOL     DestroyWindow        ( void );
    LONG_PTR GetWindowLongA       ( int nIndex );
    LONG_PTR GetWindowLongW       ( int nIndex );
    LONG_PTR SetWindowLongA       ( int nIndex, LONG_PTR lNewLong );
    LONG_PTR SetWindowLongW       ( int nIndex, LONG_PTR lNewLong );
    int      GetWindowTextA       ( PSTR pString, int nMaxCount );
    int      GetWindowTextW       ( PWSTR pString, int nMaxCount );
    BOOL     SetWindowTextA       ( PCSTR pString );
    BOOL     SetWindowTextW       ( PCWSTR pString );
    BOOL     SetWindowPos         ( HWND hWndInsAfter, int x, int y,
                                    int cx, int cy, UINT uFlags );
    LRESULT  SendMessageA         ( UINT uMsg, WPARAM wParam, LPARAM lParam );
    LRESULT  SendMessageW         ( UINT uMsg, WPARAM wParam, LPARAM lParam );
    BOOL     PostMessageA         ( UINT uMsg, WPARAM wParam, LPARAM lParam );
    BOOL     PostMessageW         ( UINT uMsg, WPARAM wParam, LPARAM lParam );
    BOOL     DispatchMessageA     ( LPMSG pMsg );
    BOOL     DispatchMessageW     ( LPMSG pMsg );
    BOOL     ProcessMessageA      ( BOOL fAllWindows,
                                    UINT wMsgFilterMin = 0,
                                    UINT wMsgFilterMax = 0,
                                    UINT wMsgTerminate = WM_QUIT );
    BOOL     ProcessMessageW      ( BOOL fAllWindows,
                                    UINT wMsgFilterMin = 0,
                                    UINT wMsgFilterMax = 0,
                                    UINT wMsgTerminate = WM_QUIT );
    BOOL     ProcessMessagesA     ( BOOL fAllWindows,
                                    UINT wMsgFilterMin = 0,
                                    UINT wMsgFilterMax = 0,
                                    UINT wMsgTerminate = WM_QUIT );
    BOOL     ProcessMessagesW     ( BOOL fAllWindows,
                                    UINT wMsgFilterMin = 0,
                                    UINT wMsgFilterMax = 0,
                                    UINT wMsgTerminate = WM_QUIT );
    void     DisplayErrorMessageA ( PCSTR pszError );
    void     DisplayErrorMessageW ( PCWSTR pszError );
    void     DisplayErrorCode     ( HRESULT hResult );
    HWND     GetHandle            ( void );
    HWND     GetItem              ( int nItemId );
    LRESULT  SendItemMessageA     ( int nItemId, UINT uMsg,
                                    WPARAM wParam, LPARAM lParam );
    LRESULT  SendItemMessageW     ( int nItemId, UINT uMsg,
                                    WPARAM wParam, LPARAM lParam );
    BOOL     EnableControl        ( int nIdCtrl, BOOL fEnable );
    BOOL     CheckRadioButton     ( int nIdFirst, int nIdLast, int nIdCheck );
    int      GetCheckedRadioButton( int nIdFirst, int nIdLast );
    BOOL     CheckButton          ( int nIdButton, UINT uCheck );
    UINT     IsButtonChecked      ( int nIdButton );
    UINT     GetItemInt           ( int nIdItem, PBOOL pfTrans,
                                    BOOL fSigned = TRUE );
    BOOL     SetItemInt           ( int nIdItem, UINT uValue,
                                    BOOL fSigned = TRUE );
    UINT     GetItemTextA         ( int nIdItem, PSTR pString, int nCount );
    UINT     GetItemTextW         ( int nIdItem, PWSTR pString, int nCount );
    BOOL     SetItemTextA         ( int nIdCtrl, PCSTR pString );
    BOOL     SetItemTextW         ( int nIdCtrl, PCWSTR pString );

    CDialogWnd();
    virtual ~CDialogWnd();

  protected:
    virtual BOOL CmdProc( WORD wId, WORD wCode, HWND hCtrl );
    virtual BOOL DlgProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

  private:
    HWND   m_hwndDlg;  // handle of dialog window
    LPARAM m_lInitPar; // initialization parameter
    BOOL   m_fIsModal; // TRUE if modal dialog box

    static INT_PTR CALLBACK StdDlgProc( HWND hwnd, UINT uMsg,
                                        WPARAM wParam, LPARAM lParam );
    static LRESULT CALLBACK DlgWndProc( HWND hwnd, UINT uMsg,
                                        WPARAM wParam, LPARAM lParam );
};

#undef CreateDialogA
#define CreateDialogA(hInstance, pName, hWndParent) \
  CreateDialogParamA(hInstance, pName, hWndParent, 0)

#undef CreateDialogW
#define CreateDialogW(hInstance, pName, hWndParent) \
  CreateDialogParamW(hInstance, pName, hWndParent, 0L)

#undef DialogBoxA
#define DialogBoxA(hInstance, pTemplate, hWndParent) \
  DialogBoxParamA(hInstance, pTemplate, hWndParent, 0L)

#undef DialogBoxW
#define DialogBoxW(hInstance, pTemplate, hWndParent) \
  DialogBoxParamW(hInstance, pTemplate, hWndParent, 0L)

#ifdef UNICODE
#define RegisterClass       RegisterClassW
#define RegisterClassEx     RegisterClassExW
#define CreateDialogParam   CreateDialogParamW
#define CreateDialog        CreateDialogW
#define DialogBoxParam      DialogBoxParamW
#define DialogBox           DialogBoxW
#define GetWindowLong       GetWindowLongW
#define SetWindowLong       SetWindowLongW
#define GetWindowText       GetWindowTextW
#define SetWindowText       SetWindowTextW
#define SendMessage         SendMessageW
#define PostMessage         PostMessageW
#define DispatchMessage     DispatchMessageW
#define ProcessMessage      ProcessMessageW
#define ProcessMessages     ProcessMessagesW
#define DisplayErrorMessage DisplayErrorMessageW
#define SendItemMessage     SendItemMessageW
#define GetItemText         GetItemTextW
#define SetItemText         SetItemTextW
#else
#define RegisterClass       RegisterClassA
#define RegisterClassEx     RegisterClassExA
#define CreateDialogParam   CreateDialogParamA
#define CreateDialog        CreateDialogA
#define DialogBoxParam      DialogBoxParamA
#define DialogBox           DialogBoxA
#define GetWindowLong       GetWindowLongA
#define SetWindowLong       SetWindowLongA
#define GetWindowText       GetWindowTextA
#define SetWindowText       SetWindowTextA
#define SendMessage         SendMessageA
#define PostMessage         PostMessageA
#define DispatchMessage     DispatchMessageA
#define ProcessMessage      ProcessMessageA
#define ProcessMessages     ProcessMessagesA
#define DisplayErrorMessage DisplayErrorMessageA
#define SendItemMessage     SendItemMessageA
#define GetItemText         GetItemTextA
#define SetItemText         SetItemTextA
#endif

//------------------------------------------------------------------------
// Function:
//  CDialogWnd::EndDialog
//
// Description:
//  The function destroys a modal dialog box, causing the system to end
//  any processing for the dialog box. To destroy a modeless dialog box
//  use DestroyWindow.
//
// Arguments:
//  nResult -> Specifies the value to be returned to the application from
//             the function that created the dialog box.
//
// Results:
//  If the function succeeds, the return value is TRUE. If the function
//  fails, the return value is FALSE. To get extended error information,
//  call GetLastError.
//------------------------------------------------------------------------
inline BOOL CDialogWnd::EndDialog(int nResult)
{
  return( ::EndDialog(m_hwndDlg, nResult) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::ShowWindow
//
// Description:
//  The function function sets the specified window's show state.
//
// Arguments:
//  nCmdShow -> Specifies how the window is to be shown. This parameter
//              can be one of the SW_xxx values.
//
// Results:
//  If the window was previously visible, the return value is nonzero.
//  If the window was previously hidden, the return value is zero. 
//------------------------------------------------------------------------
inline BOOL CDialogWnd::ShowWindow(int nCmdShow)
{
  return( ::ShowWindow(m_hwndDlg, nCmdShow) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::UpdateWindow
//
// Description:
//  This function updates the client area of the by sending a WM_PAINT
//  message to the window if the window's update region is not empty.
//  The function sends the WM_PAINT message directly to the window
//  procedure, bypassing the applications queue. If the update region
//  is empty, no message is sent. 
//
// Arguments:
//  none
//
// Results:
//  If the function succeeds, the return value is TRUE. If the function
//  fails, the return value is FALSE. To get extended error information,
//  call GetLastError.
//------------------------------------------------------------------------
inline BOOL CDialogWnd::UpdateWindow(void)
{
  return( ::UpdateWindow(m_hwndDlg) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::EnableWindow
//
// Description:
//  This function enables or disables mouse and keyboard input to the
//  dialog window. When input is disabled, the dialog window does not
//  receive input such as mouse clicks and key presses. When input is
//  enabled, the window receives all input.
//
// Arguments:
//  fEnable -> Specifies whether to enable or disable the window. If
//             this parameter is TRUE, the window is enabled. If the
//             parameter is FALSE, the window is disabled.
//
// Results:
//  If the window was previously disabled, the return value is FALSE. If
//  the window was not previously disabled, the return value is TRUE. To
//  get extended error information, call GetLastError.
//------------------------------------------------------------------------
inline BOOL CDialogWnd::EnableWindow(BOOL fEnable)
{
  return( ~::EnableWindow(m_hwndDlg, fEnable) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::DestroyWindow
//
// Description:
//  The function destroys a modeless dialog box, causing the system to
//  end any processing for the dialog box. To destroy a modal dialog box
//  use EndDialog.
//
// Arguments:
//  none
//
// Results:
//  If the function succeeds, the return value is TRUE. If the function
//  fails, the return value is FALSE. To get extended error information,
//  call GetLastError.
//------------------------------------------------------------------------
inline BOOL CDialogWnd::DestroyWindow(void)
{
  return( ::DestroyWindow(m_hwndDlg) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::GetWindowLong
//
// Description:
//  The function retrieves information about the specified window. The
//  function also retrieves the value at the specified offset into the
//  extra window memory.
//
// Arguments:
//  nIndex -> Specifies the zero-based offset to the value to be retrieved.
//            Valid values are in the range zero through the number of
//            bytes of extra window memory, minus four; for example, if
//            you specified 12 or more bytes of extra memory, a value of
//            8 would be an index to the third 32-bit integer. To retrieve
//            any other value, specify one of the following values.
//            - GWL_EXSTYLE
//              Retrieves the extended window styles. For more information,
//              see CreateWindowEx.  
//            - GWL_STYLE
//              Retrieves the window styles. 
//            - GWLP_WNDPROC
//              Retrieves the address of the window procedure, or a handle
//              representing the address of the window procedure. You must
//              use the CallWindowProc function to call the window procedure.
//            - GWLP_HINSTANCE
//              Retrieves a handle to the application instance. 
//            - GWLP_HWNDPARENT
//              Retrieves a handle to the parent window, if any.
//            - GWLP_ID
//              Retrieves the identifier of the window. 
//            - GWLP_USERDATA
//              Retrieves the 32-bit value associated with the window. Each
//              window has a corresponding 32-bit value intended for use by
//              the application that created the window. This value is
//              initially zero. 
//            The following values are also available for dialog boxes.
//            - DWLP_DLGPROC
//              Retrieves the address of the dialog box procedure, or
//              a handle representing the address of the dialog box
//              procedure. You must use the CallWindowProc function to
//              call the dialog box procedure. 
//            - DWLP_MSGRESULT
//              Retrieves the return value of a message processed in the
//              dialog box procedure. 
//            - DWLP_USER
//              Retrieves extra information private to the application,
//              such as handles or pointers. 
//
// Results:
//  If the function succeeds, the return value is the requested value.
//  If the function fails, the return value is zero. To get extended
//  error information, call GetLastError.
//------------------------------------------------------------------------
inline LONG_PTR CDialogWnd::GetWindowLongA(int nIndex)
{
  return( ::GetWindowLongPtrA(m_hwndDlg, nIndex) );
}
inline LONG_PTR CDialogWnd::GetWindowLongW(int nIndex)
{
  return( ::GetWindowLongPtrW(m_hwndDlg, nIndex) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::SetWindowLong
//
// Description:
//  The function changes an attribute of the specified window. The
//  function also sets the 32-bit (long) value at the specified
//  offset into the extra window memory.
//
// Arguments:
//  nIndex    -> Specifies the zero-based offset to the value to be set.
//               Valid values are in the range zero through the number of
//               bytes of extra window memory, minus the size of an integer.
//               For example, if you specified 12 or more bytes of extra
//               memory, a value  of 8 would be an index to the third 32-bit
//               integer. To set any other value, specify one of the following
//               values:
//               - GWL_EXSTYLE
//                 Sets a new extended window style. For more information,
//                 see CreateWindowEx. 
//               - GWL_STYLE
//                 Sets a new window style.
//               - GWLP_WNDPROC
//                 Sets a new address for the window procedure. 
//                 Windows NT/2000: You cannot change this attribute if
//                 the window does not belong to the same process as the
//                 calling thread.
//               - GWLP_HINSTANCE
//                 Sets a new application instance handle.
//               - GWLP_ID
//                 Sets a new identifier of the window.
//               - GWLP_USERDATA
//                 Sets the 32-bit value associated with the window. Each
//                 window has a corresponding 32-bit value intended for
//                 use by the application that created the window. This
//                 value is initially zero.
//               The following values are also available for dialog boxes.
//               - DWLP_DLGPROC
//                 Sets the new address of the dialog box procedure.
//               - DWLP_MSGRESULT
//                 Sets the return value of a message processed in the
//                 dialog box procedure.
//               - DWLP_USER
//                 Sets new extra information that is private to the
//                 application, such as handles or pointers.
//  dwNewLong -> Specifies the replacement value. 
//
// Results:
//  If the function succeeds, the return value is the previous value of
//  the specified offset. If the function fails, the return value is zero.
//  To get extended error information, call GetLastError. If the previous
//  value is zero and the function succeeds, the return value is zero and
//  the result from GetLastError is also zero.
//------------------------------------------------------------------------
inline LONG_PTR CDialogWnd::SetWindowLongA(int nIndex, LONG_PTR lNewLong)
{
  SetLastError(0);
  return( ::SetWindowLongPtrA(m_hwndDlg, nIndex, lNewLong) );
}
inline LONG_PTR CDialogWnd::SetWindowLongW(int nIndex, LONG_PTR lNewLong)
{
  SetLastError(0);
  return( ::SetWindowLongPtrW(m_hwndDlg, nIndex, lNewLong) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::GetWindowText
//
// Description:
//  The function copies the text of the window's title bar (if it has
//  one) into a buffer.
//
// Arguments:
//  pszString <- Pointer to the buffer that will receive the text.
//  nMaxCount -> Specifies the maximum number of characters to copy to
//               the buffer, including the NULL character. If the text
//               exceeds this limit, it is truncated.
//
// Results:
//  If the function succeeds, the return value is the length, in
//  characters, of the copied string, not including the terminating
//  null character. If the window has no title bar or text, if the
//  title bar is empty, the return value is zero. To get extended
//  error information, call GetLastError.
//------------------------------------------------------------------------
inline int CDialogWnd::GetWindowTextA(PSTR pszString, int nMaxCount)
{
  return( ::GetWindowTextA(m_hwndDlg, pszString, nMaxCount) );
}
inline int CDialogWnd::GetWindowTextW(PWSTR pszString, int nMaxCount)
{
  return( ::GetWindowTextW(m_hwndDlg, pszString, nMaxCount) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::SetWindowText
//
// Description:
//  The function changes the text of the window's title bar (if it has
//  one).
//
// Arguments:
//  pszString -> Pointer to a null-terminated string to be used as the
//               new title text. 
//
// Results:
//  If the function succeeds, the return value is nonzero. If the
//  function fails, the return value is zero. To get extended error
//  information, call GetLastError.
//------------------------------------------------------------------------
inline BOOL CDialogWnd::SetWindowTextA(PCSTR pszString)
{
  return( ::SetWindowTextA(m_hwndDlg, pszString) );
}
inline BOOL CDialogWnd::SetWindowTextW(PCWSTR pszString)
{
  return( ::SetWindowTextW(m_hwndDlg, pszString) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::SetWindowPos
//
// Description:
//  The function changes the size, position, and Z order of the dialog
//  window. The dialog window is ordered according to the appearance on
//  the screen. The topmost window receives the highest rank and is the
//  first window in the Z order.
//
// Arguments:
//  hWndInsAfter -> Handle to the window to precede the positioned window
//                  in the Z order. This parameter must be a window handle
//                  or one of the following values.
//                  * HWND_BOTTOM
//                    Places the window at the bottom of the Z order. If
//                    the dialog window is a topmost window, the window
//                    loses its topmost status and is placed at the bottom
//                    of all other windows.
//                  * HWND_NOTOPMOST
//                    Places the window above all non-topmost windows
//                    (that is, behind all topmost windows). This flag
//                    has no effect if the window is already a non-topmost
//                    window.
//                  * HWND_TOP
//                    Places the window at the top of the Z order.
//                  * HWND_TOPMOST
//                    Places the window above all non-topmost windows.
//                    The window maintains its topmost position even
//                    when it is deactivated.
//  x            -> Specifies the new position of the left side of the
//                  window, in client coordinates. 
//  y            -> Specifies the new position of the top of the window,
//                  in client coordinates. 
//  cx           -> Specifies the new width of the window, in pixels.
//  cy           -> Specifies the new height of the window, in pixels. 
//  uFlags       -> Specifies the window sizing and positioning flags.
//                  This parameter can be a combination of the SWP_
//                  constants. 
//
// Results:
//  If the function succeeds, the return value is nonzero. If the
//  function fails, the return value is zero. To get extended error
//  information, call GetLastError.
//------------------------------------------------------------------------
inline BOOL CDialogWnd::SetWindowPos(HWND hWndInsAfter, int x, int y,
                                         int cx, int cy, UINT uFlags)
{
  return( ::SetWindowPos(m_hwndDlg, hWndInsAfter, x, y, cx, cy, uFlags ) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::SendMessage
//
// Description:
//  The function sends a message to the dialog box window. It calls the
//  dialog procedure for the specified dialog box and does not return
//  until the window procedure has processed the message.
//
// Arguments:
//  uMsg    -> Specifies the message to be sent. 
//  wParam  -> Specifies additional message-specific information. 
//  lParam  -> Specifies additional message-specific information. 
//
// Results:
//  The return value specifies the result of the message processing;
//  it depends on the message sent.
//------------------------------------------------------------------------
inline LRESULT CDialogWnd::SendMessageA(UINT   uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
  return( ::SendMessageA(m_hwndDlg, uMsg, wParam, lParam) );
}
inline LRESULT CDialogWnd::SendMessageW(UINT   uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
  return( ::SendMessageW(m_hwndDlg, uMsg, wParam, lParam) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::PostMessage
//
// Description:
//  The function places (posts) a message in the message queue associated
//  with the thread that created the dialog box window and returns without
//  waiting for the thread to process the message.
//
// Arguments:
//  uMsg    -> Specifies the message to be posted.
//  wParam  -> Specifies additional message-specific information. 
//  lParam  -> Specifies additional message-specific information. 
//
// Results:
//  If the function succeeds, the return value is TRUE. If the function
//  fails, the return value is FALSE. To get extended error information,
//  call GetLastError. 
//------------------------------------------------------------------------
inline BOOL CDialogWnd::PostMessageA(UINT   uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam)
{
  return( ::PostMessageA(m_hwndDlg, uMsg, wParam, lParam) );
}
inline BOOL CDialogWnd::PostMessageW(UINT   uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam)
{
  return( ::PostMessageW(m_hwndDlg, uMsg, wParam, lParam) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::GetHandle
//
// Description:
//  The function retreive the handle of the dialog box window.
//
// Arguments:
//  none
//
// Results:
//  If the function succeeds, the return value is the HWND of the
//  dialog box window. If the function fails, the return value is NULL.
//------------------------------------------------------------------------
inline HWND CDialogWnd::GetHandle(void)
{
  return( m_hwndDlg );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::GetItem
//
// Description:
//  The function retrieves a handle to a control in the dialog box.
//
// Arguments:
//  nItemId -> Specifies the identifier of the control to be retrieved
//
// Results:
//  If the function succeeds, the return value is the window handle of
//  the specified control. If the function fails, the return value is
//  NULL, indicating an a nonexistent control. To get extended error
//  information, call GetLastError.
//------------------------------------------------------------------------
inline HWND CDialogWnd::GetItem(int nItemId)
{
  return( ::GetDlgItem(m_hwndDlg, nItemId) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::SendItemMessage
//
// Description:
//  The function sends a message to the specified control in the
//  dialog box. 
//
// Arguments:
//  nItemId -> Specifies the identifier of the control that receives
//             the message
//  uMsg    -> Specifies the message to be sent. 
//  wParam  -> Specifies additional message-specific information. 
//  lParam  -> Specifies additional message-specific information. 
//
// Results:
//  The return value specifies the result of the message processing
//  and depends on the message sent.
//------------------------------------------------------------------------
inline LRESULT CDialogWnd::SendItemMessageA(int    nItemId,
                                            UINT   uMsg,
                                            WPARAM wParam,
                                            LPARAM lParam)
{
  return( ::SendDlgItemMessageA(m_hwndDlg, nItemId, uMsg, wParam, lParam) );
}
inline LRESULT CDialogWnd::SendItemMessageW(int    nItemId,
                                            UINT   uMsg,
                                            WPARAM wParam,
                                            LPARAM lParam)
{
  return( ::SendDlgItemMessageW(m_hwndDlg, nItemId, uMsg, wParam, lParam) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::EnableControl
//
// Description:
//  This function enables or disables mouse and keyboard input to the
//  specified control. When input is disabled, the control does not
//  receive input such as mouse clicks and key presses. When input is
//  enabled, the control receives all input. 
//
// Arguments:
//  nIDCtrl -> Specifies the identifier of the control. 
//  fEnable -> Specifies whether to enable or disable the control.
//             If this parameter is TRUE, the control is enabled.
//             If the parameter is FALSE, the control is disabled. 
//
// Results:
//  If the control was previously disabled, the return value is FALSE. If
//  the control was not previously disabled, the return value is TRUE. To
//  get extended error information, call GetLastError.
//------------------------------------------------------------------------
inline BOOL CDialogWnd::EnableControl(int nIDCtrl, BOOL fEnable)
{
  HWND hCtrl;
  
  hCtrl = ::GetDlgItem(m_hwndDlg, nIDCtrl);
  return( ~::EnableWindow(hCtrl, fEnable) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::CheckRadioButton
//
// Description:
//  The function adds a check mark to (checks) a specified radio button
//  in a group and removes a check mark from (clears) all other radio
//  buttons in the group.
//
// Arguments:
//  nIDFirst -> Specifies the identifier of the first radio button in
//              the group. 
//  nIDLast  -> Specifies the identifier of the last radio button in
//              the group. 
//  nIDCheck -> Specifies the identifier of the radio button to select. 
//
// Results:
//  If the function succeeds, the return value is nonzero.
//  If the function fails, the return value is zero. To get
//  extended error information, call GetLastError. 
//------------------------------------------------------------------------
inline BOOL CDialogWnd::CheckRadioButton(int nIDFirst,
                                         int nIDLast,
                                         int nIDCheck)
{
  return( ::CheckRadioButton(m_hwndDlg, nIDFirst, nIDLast, nIDCheck) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::GetCheckedRadioButton
//
// Description:
//  The function retrieves the ID of the checked button in a
//  group of radio buttons.
//
// Arguments:
//  nIDFirst -> Specifies the identifier of the first radio button in
//              the group. 
//  nIDLast  -> Specifies the identifier of the last radio button in
//              the group. 
//
// Results:
//  If the function succeeds, the return value is the ID of the checked
//  ratio button. If the function fails, the return value is -1. To get
//  extended error information, call GetLastError. 
//------------------------------------------------------------------------
inline int CDialogWnd::GetCheckedRadioButton(int nIDFirst, int nIDLast)
{
  int i;

  if (nIDLast < nIDFirst)
  {
    i = nIDFirst;
    nIDFirst = nIDLast;
    nIDLast = i;
  }

  for (i = nIDFirst; i <= nIDLast; i++)
  {
    if (::IsDlgButtonChecked(m_hwndDlg, i))
      return(i);
  }

  return( -1 );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::CheckButton
//
// Description:
//  The function function changes the check state of a button control.
//
// Arguments:
//  nIDButton -> Specifies the identifier of the button to modify.
//  uCheck    -> Specifies the check state of the button. This
//               parameter can be one of the following values:
//               * BST_CHECKED
//                 Sets the button state to checked.
//               * BST_INDETERMINATE
//                 Sets the button state to grayed, indicating an
//                 indeterminate state. Use this value only if the
//                 button has the BS_3STATE or BS_AUTO3STATE style.
//               * BST_UNCHECKED
//                 Sets the button state to cleared 
//
// Results:
//  If the function succeeds, the return value is nonzero.
//  If the function fails, the return value is zero. To get
//  extended error information, call GetLastError. 
//
// Remarks:
//  The function sends a BM_SETCHECK message to the specified button
//  control.
//------------------------------------------------------------------------
inline BOOL CDialogWnd::CheckButton(int nIDButton, UINT uCheck)
{
  return( ::CheckDlgButton(m_hwndDlg, nIDButton, uCheck) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::IsButtonChecked
//
// Description:
//  The function adds a check mark to (checks) a specified radio button
//  in a group and removes a check mark from (clears) all other radio
//  buttons in the group.
//
// Arguments:
//  nIDButton -> Specifies the identifier of the button control.
//
// Results:
//  The return value from a button created with the BS_AUTOCHECKBOX,
//  BS_AUTORADIOBUTTON, BS_AUTO3STATE, BS_CHECKBOX, BS_RADIOBUTTON,
//  or BS_3STATE style can be one of the following. 
//  * BST_CHECKED
//    Button is checked. 
//  * BST_INDETERMINATE
//    Button is grayed, indicating an indeterminate state (applies only
//    if the button has the BS_3STATE or BS_AUTO3STATE style). 
//  * BST_UNCHECKED
//    Button is cleared.
//
//  If the button has any other style, the return value is zero.
//------------------------------------------------------------------------
inline UINT CDialogWnd::IsButtonChecked(int nIDButton)
{
  return( ::IsDlgButtonChecked(m_hwndDlg, nIDButton) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::GetItemInt
//
// Description:
//  The function translates the text of a specified control in the
//  dialog box into an integer value.
//
// Arguments:
//  nIdItem -> Specifies the identifier of the control whose text is
//             to be translated. 
//  pfTrans <- Pointer to a variable that receives a success or failure
//             value (TRUE indicates success, FALSE indicates failure). 
//             If this parameter is NULL, the function returns no
//             information about success or failure. 
//  fSigned -> Specifies whether the function should examine the text
//             for a minus sign at the beginning and return a signed
//             integer value if it finds one (TRUE specifies this should
//             be done, FALSE that it should not).
//
// Results:
//  If the function succeeds, the variable pointed to by pfTrans is set
//  to TRUE, and the return value is the translated value of the control
//  text. If the function fails, the variable pointed to by pfTrans is
//  set to FALSE, and the return value is zero. Note that, since zero is
//  a possible translated value, a return value of zero does not by itself
//  indicate failure.
//  If pfTrans is NULL, the function returns no information about success
//  or failure. If the fSigned parameter is TRUE, specifying that the value
//  to be retrieved is a signed integer value, cast the return value to an
//  int type. To get extended error information, call GetLastError.
//
// Remarks:
//  The function retrieves the text of the specified control by sending
//  a WM_GETTEXT message to the control. The function translates the
//  retrieved text by stripping any extra spaces at the beginning of
//  the text and then converting the decimal digits. The function stops
//  translating when it reaches the end of the text or encounters a
//  non-numeric character. The function returns zero if the translated
//  value is greater than INT_MAX (for signed numbers) or UINT_MAX
//  (for unsigned numbers).
//------------------------------------------------------------------------
inline UINT CDialogWnd::GetItemInt(int   nIdItem,
                                   PBOOL pfTrans,
                                   BOOL  fSigned)
{
  return( ::GetDlgItemInt(m_hwndDlg, nIdItem, pfTrans, fSigned) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::SetItemInt
//
// Description:
//  The function sets the text of a control in the dialog box to the
//  string representation of a specified integer value.
//
// Arguments:
//  nIdItem -> Specifies the control to be changed. 
//  uValue  -> Specifies the integer value used to generate the item
//             text.
//  fSigned -> Specifies whether the uValue parameter is signed or
//             unsigned. If this parameter is TRUE, uValue is signed.
//             If this parameter is TRUE and uValue is less than zero,
//             a minus sign is placed before the first digit in the
//             string. If this parameter is FALSE, uValue is unsigned. 
//
// Results:
//  If the function succeeds, the return value is nonzero. If the
//  function fails, the return value is zero. To get extended error
//  information, call GetLastError.
//
// Remarks:
//  To set the new text, this function sends a WM_SETTEXT message to
//  the specified control.
//------------------------------------------------------------------------
inline BOOL CDialogWnd::SetItemInt(int nIdItem, UINT uValue, BOOL fSigned)
{
  return( ::SetDlgItemInt(m_hwndDlg, nIdItem, uValue, fSigned) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::GetItemText
//
// Description:
//  The function 
//  in the dialog box. 
//
// Arguments:
//  nIdItem -> Specifies the identifier of the control whose title or
//             text is to be retrieved.
//  pString <- Pointer to the buffer to receive the title or text.
//             the text to be copied to the control.
//  nCount  -> Specifies the maximum length, in TCHARs, of the string
//             to be copied to the buffer pointed to by pString. If
//             the length of the string exceeds the limit, the string
//             is truncated. 
//
// Results:
//  If the function succeeds, the return value specifies the number of
//  TCHARs copied to the buffer, not including the terminating null
//  character. If the function fails, the return value is zero. To get
//  extended error information, call GetLastError.
//
// Remarks:
//  To set the new text, this function sends a WM_SETTEXT message to
//  the specified control.
//------------------------------------------------------------------------
inline UINT CDialogWnd::GetItemTextA(int  nIdItem,
                                     PSTR pString,
                                     int  nCount)
{
  return( ::GetDlgItemTextA(m_hwndDlg, nIdItem, pString, nCount) );
}
inline UINT CDialogWnd::GetItemTextW(int   nIdItem,
                                     PWSTR pString,
                                     int   nCount)
{
  return( ::GetDlgItemTextW(m_hwndDlg, nIdItem, pString, nCount) );
}
//------------------------------------------------------------------------
// Function:
//  CDialogWnd::SetItemText
//
// Description:
//  The function sets the title or text of a control in the dialog box.
//
// Arguments:
//  nIdItem -> Specifies the control with a title or text to be set.
//  pString -> Pointer to the null-terminated string that contains
//             the text to be copied to the control.
//
// Results:
//  If the function succeeds, the return value is nonzero. If the
//  function fails, the return value is zero. To get extended error
//  information, call GetLastError.
//
// Remarks:
//  To set the new text, this function sends a WM_SETTEXT message to
//  the specified control.
//------------------------------------------------------------------------
inline BOOL CDialogWnd::SetItemTextA(int nIdItem, PCSTR pString)
{
  return( ::SetDlgItemTextA(m_hwndDlg, nIdItem, pString) );
}
inline BOOL CDialogWnd::SetItemTextW(int nIdItem, PCWSTR pString)
{
  return( ::SetDlgItemTextW(m_hwndDlg, nIdItem, pString) );
}

#endif // __cplusplus
#endif //_DIALOG_HPP_
