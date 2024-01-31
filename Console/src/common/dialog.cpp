//////////////////////////////////////////////////////////////////////////
// HMS Technology Center Ravensburg GmbH
//////////////////////////////////////////////////////////////////////////
/**

  Implementation of the CDialogWnd class.

*/
//////////////////////////////////////////////////////////////////////////
// Copyright (C) 2001-2019
// HMS Technology Center Ravensburg GmbH, all rights reserved
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "dialog.hpp"

#ifdef _MSC_VER
#pragma warning(disable:4100)     // unreferenced formal parameter
#endif

//////////////////////////////////////////////////////////////////////////
// local data types
//////////////////////////////////////////////////////////////////////////

typedef struct _DLGPARA
{
  CDialogWnd* pdlg;    // pointer to the dialog object
  int         minx;    // width of smallest control
  int         miny;    // height of smallest control
  int         dlgw;    // width of dialog window
  int         dlgh;    // height of dialog window
  int         winw;    // current width of dialog window
  int         winh;    // current height of dialog window
  int         oldx;    // last horizontal scroll position
  int         oldy;    // last vertical scroll position
} DLGPARA, *PDLGPARA;



/*##########################################################################*/
/*### static helper functions                                            ###*/
/*##########################################################################*/


/*****************************************************************************
 Function:
  GetSmallestDlgCtrl

 Description:
  This function retrieves the dimensions of the smallest control in the
  dialog window.

 Arguments:
  hwnd -> Handle to the dialog window.

 Results:
  The function returns the width in the LOWORD and the height in the
  HIWORD of the return value.
*****************************************************************************/
static DWORD GetSmallestDlgCtrl(HWND hwnd)
{
  HWND hctl;
  RECT rect;
  WORD w, h;

  hctl = ::GetWindow(hwnd, GW_CHILD);

  w = 0xFFFF;
  h = 0xFFFF;

  while (hctl)
  {
    ::GetWindowRect(hctl, &rect);
    w = min(w, (WORD) (rect.right - rect.left));
    h = min(h, (WORD) (rect.bottom - rect.top));
    hctl = ::GetWindow(hctl, GW_HWNDNEXT);
  }

  if (!w || (w == 0xFFFF)) w = 1;
  if (!h || (w == 0xFFFF)) h = 1;

  return( MAKELONG(w, h) );
}
/*****************************************************************************
 End of GetSmallestDlgCtrl
*****************************************************************************/


/*****************************************************************************
 Function:
  MoveDlgCtrls

 Description:
  This function moves the position off all controls in the dialog window
  relative to the current control position.

 Parameter:
  hwnd -> Handle to the dialog window.
  dx   -> relative horizontal movement in pixels
  dy   -> relative vertical movement in pixels

 Results:
  none
*****************************************************************************/
static void MoveDlgCtrls(HWND hwnd, int dx, int dy)
{
  RECT dlgr;
  HWND hctl;
  RECT ctlr;
  int  x, y;

  if (!dx && !dy) return;

  ::GetClientRect(hwnd, &dlgr);
  ::ClientToScreen(hwnd, (LPPOINT) &dlgr);

  hctl = ::GetWindow(hwnd, GW_CHILD);

  while (hctl)
  {
    ::GetWindowRect(hctl, &ctlr);

    x = (ctlr.left - dlgr.left) + dx;
    y = (ctlr.top  - dlgr.top ) + dy;

    ::SetWindowPos(hctl,NULL,x,y,0,0,SWP_NOZORDER|SWP_NOSIZE);
    hctl = ::GetWindow(hctl, GW_HWNDNEXT);
  }

  ::InvalidateRect(hwnd, NULL, TRUE);
  ::UpdateWindow(hwnd);
}
/*****************************************************************************
 End of MoveDlgCtrls
*****************************************************************************/


/*##########################################################################*/
/*### Methods for CDialogWnd class                                       ###*/
/*##########################################################################*/


/*****************************************************************************
 Function:
  CDialogWnd::CDialogWnd

 Description:
  Default constructor for a new modal dialog box object.

 Arguments:
  none

 Results:
  none
*****************************************************************************/
CDialogWnd::CDialogWnd()
{
  m_hwndDlg  = NULL;
  m_lInitPar = 0;
  m_fIsModal = FALSE;
}
/*****************************************************************************
 End of CDialogWnd::CDialogWnd
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::~CDialogWnd

 Description:
  Destructor for a modal dialog box object.

 Arguments:
  none

 Results:
  none
*****************************************************************************/
CDialogWnd::~CDialogWnd()
{
  if (m_hwndDlg != NULL)
  {
    EndDialog();
    DestroyWindow();
  }

  m_hwndDlg  = NULL;
  m_lInitPar = 0;
  m_fIsModal = FALSE;
}
/*****************************************************************************
 End of CDialogWnd::~CDialogWnd
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::RegisterClassA

 Description:
  The function registers the window class used by CDialogWnd windows. Note
  that the function has been superseded by the RegisterClassExA function.
  You can still use this function, however, if you do not need to set the
  class small icon.

 Arguments:
  pWndClass -> Pointer to a WNDCLASSA structure. You must fill the structure
               with the appropriate class attributes before passing it to
               the function.

 Results:
  If the function succeeds, it returns TRUE. If the function fails,
  the return value is FALSE. To get extended error information, call
  GetLastError.
*****************************************************************************/
BOOL CDialogWnd::RegisterClassA(PWNDCLASSA pWndClass)
{
  BOOL      fResult;
  WNDCLASSA sWndCls;

  if (pWndClass != NULL)
  {
    fResult = ::GetClassInfoA(NULL, (PCSTR) WC_DIALOG, &sWndCls);
    if (fResult)
    {
      pWndClass->cbClsExtra  = sWndCls.cbClsExtra;
      pWndClass->cbWndExtra  = sWndCls.cbWndExtra;
      pWndClass->lpfnWndProc = CDialogWnd::DlgWndProc;
      fResult = (::RegisterClassA(pWndClass) != 0);
    }
  }
  else
  {
    ::SetLastError(ERROR_INVALID_PARAMETER);
    fResult = FALSE;
  }

  return( fResult );
}
/*****************************************************************************
 End of CDialogWnd::RegisterClassA
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::RegisterClassW

 Description:
  The function registers the window class used by CDialogWnd windows. Note
  that the function has been superseded by the RegisterClassExW function.
  You can still use this function, however, if you do not need to set the
  class small icon.

 Arguments:
  pWndClass -> Pointer to a WNDCLASSA structure. You must fill the structure
               with the appropriate class attributes before passing it to
               the function.

 Results:
  If the function succeeds, it returns TRUE. If the function fails,
  the return value is FALSE. To get extended error information, call
  GetLastError.
*****************************************************************************/
BOOL CDialogWnd::RegisterClassW(PWNDCLASSW pWndClass)
{
  BOOL      fResult;
  WNDCLASSW sWndCls;

  if (pWndClass != NULL)
  {
    fResult = ::GetClassInfoW(NULL, (PCWSTR) WC_DIALOG, &sWndCls);
    if (fResult)
    {
      pWndClass->cbClsExtra  = sWndCls.cbClsExtra;
      pWndClass->cbWndExtra  = sWndCls.cbWndExtra;
      pWndClass->lpfnWndProc = CDialogWnd::DlgWndProc;
      fResult = (::RegisterClassW(pWndClass) != 0);
    }
  }
  else
  {
    ::SetLastError(ERROR_INVALID_PARAMETER);
    fResult = FALSE;
  }

  return( fResult );
}
/*****************************************************************************
 End of CDialogWnd::RegisterClassW
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::RegisterClassExA

 Description:
  The function registers the window class used by CDialogWnd windows.

 Arguments:
  pWndClsEx -> Pointer to a WNDCLASSEXA structure. You must fill the
               structure with the appropriate class attributes before
               passing it to the function.

 Results:
  If the function succeeds, it returns TRUE. If the function fails,
  the return value is FALSE. To get extended error information, call
  GetLastError.
*****************************************************************************/
BOOL CDialogWnd::RegisterClassExA(PWNDCLASSEXA pWndClsEx)
{
  BOOL        fResult;
  WNDCLASSEXA sWndCls;

  if (pWndClsEx != NULL)
  {
    sWndCls.cbSize = sizeof(WNDCLASSEXA);
    fResult = ::GetClassInfoExA(NULL, (PCSTR) WC_DIALOG, &sWndCls);
    if (fResult)
    {
      pWndClsEx->cbSize      = sizeof(WNDCLASSEXA);
      pWndClsEx->cbClsExtra  = sWndCls.cbClsExtra;
      pWndClsEx->cbWndExtra  = sWndCls.cbWndExtra;
      pWndClsEx->lpfnWndProc = CDialogWnd::DlgWndProc;
      fResult = (::RegisterClassExA(pWndClsEx) != 0);
    }
  }
  else
  {
    ::SetLastError(ERROR_INVALID_PARAMETER);
    fResult = FALSE;
  }

  return( fResult );
}
/*****************************************************************************
 End of CDialogWnd::RegisterClassExA
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::RegisterClassExW

 Description:
  The function registers the window class used by CDialogWnd windows.

 Arguments:
  pWndClsEx -> Pointer to a WNDCLASSEXW structure. You must fill the
               structure with the appropriate class attributes before
               passing it to the function.

 Results:
  If the function succeeds, it returns TRUE. If the function fails,
  the return value is FALSE. To get extended error information, call
  GetLastError.
*****************************************************************************/
BOOL CDialogWnd::RegisterClassExW(PWNDCLASSEXW pWndClsEx)
{
  BOOL        fResult;
  WNDCLASSEXW sWndCls;

  if (pWndClsEx != NULL)
  {
    sWndCls.cbSize = sizeof(WNDCLASSEXW);
    fResult = ::GetClassInfoExW(NULL, (PCWSTR) WC_DIALOG, &sWndCls);
    if (fResult)
    {
      pWndClsEx->cbSize      = sizeof(WNDCLASSEXW);
      pWndClsEx->cbClsExtra  = sWndCls.cbClsExtra;
      pWndClsEx->cbWndExtra  = sWndCls.cbWndExtra;
      pWndClsEx->lpfnWndProc = CDialogWnd::DlgWndProc;
      fResult = (::RegisterClassExW(pWndClsEx) != 0);
    }
  }
  else
  {
    ::SetLastError(ERROR_INVALID_PARAMETER);
    fResult = FALSE;
  }

  return( fResult );
}
/*****************************************************************************
 End of CDialogWnd::RegisterClassExW
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::DialogBoxParamA

 Description:
  This function creates a modal dialog box from a dialog box template
  resource. Before displaying the dialog box, the function passes an
  application-defined value to the dialog box procedure as the lParam
  parameter of the WM_INITDIALOG message. An application can use this
  value to initialize dialog box controls.

 Arguments:
  hInstance     -> Handle to the module whose executable file contains the
                   dialog box template. 
  pTemplateName -> Specifies the dialog box template. This parameter is
                   either the pointer to a null-terminated character string
                   that specifies the name of the dialog box template or an
                   integer value that specifies the resource identifier of
                   the dialog box template. If the parameter specifies a
                   resource identifier, its high-order word must be zero and
                   its low-order word must contain the identifier. You can
                    use the MAKEINTRESOURCE macro to create this value.
  hWndParent    -> Handle to the window that owns the dialog box.
  lInitParam    -> Specifies the value to pass to the dialog box in the
                   lParam parameter of the WM_INITDIALOG message. 

 Results:
  If the function succeeds, the return value is the value of the nResult
  parameter specified in the call to the Destroy function used to terminate
  the dialog box. If the function fails because the hWndParent parameter is
  invalid, the return value is zero. The function returns zero in this case
  for compatibility with previous versions of Windows. If the function fails
  for any other reason, the return value is -1. To get extended error
  information, call GetLastError.
*****************************************************************************/
INT_PTR CDialogWnd::DialogBoxParamA(HINSTANCE hInstance, PCSTR pTemplateName,
                                    HWND hwndParent, LPARAM lInitParam)
{
  INT_PTR iResult;

  m_hwndDlg  = NULL;
  m_lInitPar = lInitParam;
  m_fIsModal = TRUE;

  iResult = ::DialogBoxParamA(hInstance, pTemplateName, hwndParent,
                              CDialogWnd::StdDlgProc, (LPARAM) this);

  return( iResult );
}
/*****************************************************************************
 End of CDialogWnd::DialogBoxParamA
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::DialogBoxParamW

 Description:
  This function creates a modal dialog box from a dialog box template
  resource. Before displaying the dialog box, the function passes an
  application-defined value to the dialog box procedure as the lParam
  parameter of the WM_INITDIALOG message. An application can use this
  value to initialize dialog box controls.

 Arguments:
  hInstance     -> Handle to the module whose executable file contains the
                   dialog box template. 
  pTemplateName -> Specifies the dialog box template. This parameter is
                   either the pointer to a null-terminated character string
                   that specifies the name of the dialog box template or an
                   integer value that specifies the resource identifier of
                   the dialog box template. If the parameter specifies a
                   resource identifier, its high-order word must be zero and
                   its low-order word must contain the identifier. You can
                    use the MAKEINTRESOURCE macro to create this value.
  hWndParent    -> Handle to the window that owns the dialog box.
  lInitParam    -> Specifies the value to pass to the dialog box in the
                   lParam parameter of the WM_INITDIALOG message. 

 Results:
  If the function succeeds, the return value is the value of the nResult
  parameter specified in the call to the Destroy function used to terminate
  the dialog box. If the function fails because the hWndParent parameter is
  invalid, the return value is zero. The function returns zero in this case
  for compatibility with previous versions of Windows. If the function fails
  for any other reason, the return value is -1. To get extended error
  information, call GetLastError.
*****************************************************************************/
INT_PTR CDialogWnd::DialogBoxParamW(HINSTANCE hInstance, PCWSTR pTemplateName,
                                    HWND hwndParent, LPARAM lInitParam)
{
  INT_PTR iResult;

  m_hwndDlg  = NULL;
  m_lInitPar = lInitParam;
  m_fIsModal = TRUE;

  iResult = ::DialogBoxParamW(hInstance, pTemplateName, hwndParent,
                              CDialogWnd::StdDlgProc, (LPARAM) this);

  return( iResult );
}
/*****************************************************************************
 End of CDialogWnd::DialogBoxParamW
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::CreateDialogParamA

 Description:
  This function creates a modeless dialog box from a dialog box template
  resource. Before displaying the dialog box, the function passes an
  application-defined value to the dialog box procedure as the lParam
  parameter of the WM_INITDIALOG message. An application can use this
  value to initialize dialog box controls. 

 Arguments:
  hInstance     -> Handle to the module whose executable file contains the
                   dialog box template. 
  pTemplateName -> Specifies the dialog box template. This parameter is
                   either the pointer to a null-terminated character string
                   that specifies the name of the dialog box template or an
                   integer value that specifies the resource identifier of
                   the dialog box template. If the parameter specifies a
                   resource identifier, its high-order word must be zero and
                   its low-order word must contain the identifier. You can
                    use the MAKEINTRESOURCE macro to create this value.
  hWndParent    -> Handle to the window that owns the dialog box.
  lInitParam    -> Specifies the value to pass to the dialog box in the
                   lParam parameter of the WM_INITDIALOG message. 

 Results:
  If the function succeeds, the return value is TRUE. If the function
  fails, the return value is FALSE. To get extended error information,
  call GetLastError.
*****************************************************************************/
BOOL CDialogWnd::CreateDialogParamA(HINSTANCE hInstance, PCSTR pTemplateName,
                                    HWND hwndParent, LPARAM lInitParam)
{
  m_lInitPar = lInitParam;
  m_fIsModal = TRUE;
  m_hwndDlg  = ::CreateDialogParamA(hInstance, pTemplateName,
                   hwndParent, CDialogWnd::StdDlgProc, (LPARAM) this);

  return( m_hwndDlg != NULL );
}
/*****************************************************************************
 End of CDialogWnd::CreateDialogParamA
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::CreateDialogParamW

 Description:
  This function creates a modeless dialog box from a dialog box template
  resource. Before displaying the dialog box, the function passes an
  application-defined value to the dialog box procedure as the lParam
  parameter of the WM_INITDIALOG message. An application can use this
  value to initialize dialog box controls. 

 Arguments:
  hInstance     -> Handle to the module whose executable file contains the
                   dialog box template. 
  pTemplateName -> Specifies the dialog box template. This parameter is
                   either the pointer to a null-terminated character string
                   that specifies the name of the dialog box template or an
                   integer value that specifies the resource identifier of
                   the dialog box template. If the parameter specifies a
                   resource identifier, its high-order word must be zero and
                   its low-order word must contain the identifier. You can
                    use the MAKEINTRESOURCE macro to create this value.
  hWndParent    -> Handle to the window that owns the dialog box.
  lInitParam    -> Specifies the value to pass to the dialog box in the
                   lParam parameter of the WM_INITDIALOG message. 

 Results:
  If the function succeeds, the return value is TRUE. If the function
  fails, the return value is FALSE. To get extended error information,
  call GetLastError.
*****************************************************************************/
BOOL CDialogWnd::CreateDialogParamW(HINSTANCE hInstance, PCWSTR pTemplateName,
                                    HWND hwndParent, LPARAM lInitParam)
{
  m_lInitPar = lInitParam;
  m_fIsModal = TRUE;
  m_hwndDlg  = ::CreateDialogParamW(hInstance, pTemplateName,
                   hwndParent, CDialogWnd::StdDlgProc, (LPARAM) this);

  return( m_hwndDlg != NULL );
}
/*****************************************************************************
 End of CDialogWnd::CreateDialogParamW
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::DispatchMessageA

 Description:
  This function determines whether a message is intended for
  the dialog box and, if it is, processes the message. 

 Arguments:
  pMsg -> Pointer to an MSG structure that contains the message
          to be checked.

 Results:
  If the message has been processed, the return value is TRUE.
  If the message has not been processed, the return value is
  FALSE. To get extended error information, call GetLastError.
*****************************************************************************/
BOOL CDialogWnd::DispatchMessageA(LPMSG pMsg)
{
  BOOL fResult;

  if (m_hwndDlg != NULL)
  {
    if (::IsDialogMessageA(m_hwndDlg, pMsg))
    {
      fResult = TRUE;
    }
    else if (pMsg->hwnd == m_hwndDlg)
    {
      ::TranslateMessage(pMsg);
      ::DispatchMessageA(pMsg);
      fResult = TRUE;
    }
    else
    {
      fResult = FALSE;
    }
  }
  else
  {
    fResult = FALSE;
  }

  return( fResult );
}
/*****************************************************************************
 End of CDialogWnd::DispatchMessageA
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::DispatchMessageW

 Description:
  This function determines whether a message is intended for
  the dialog box and, if it is, processes the message. 

 Arguments:
  pMsg -> Pointer to an MSG structure that contains the message
          to be checked.

 Results:
  If the message has been processed, the return value is TRUE.
  If the message has not been processed, the return value is
  FALSE. To get extended error information, call GetLastError.
*****************************************************************************/
BOOL CDialogWnd::DispatchMessageW(LPMSG pMsg)
{
  BOOL fResult;

  if (m_hwndDlg != NULL)
  {
    if (::IsDialogMessageW(m_hwndDlg, pMsg))
    {
      fResult = TRUE;
    }
    else if (pMsg->hwnd == m_hwndDlg)
    {
      ::TranslateMessage(pMsg);
      ::DispatchMessageW(pMsg);
      fResult = TRUE;
    }
    else
    {
      fResult = FALSE;
    }
  }
  else
  {
    fResult = FALSE;
  }

  return( fResult );
}
/*****************************************************************************
 End of CDialogWnd::DispatchMessageW
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::ProcessMessageA

 Description:
  This function checks the thread message queue for a posted message
  intended either for the dialog box, or any other window, and process
  a single message (if any exist).

 Arguments:
  fAllWindows   -> If set to TRUE the function process all messages for
                   all windows of the thread. If set to FALSE the function
                   processes only those messages intended for the dialog box.
  wMsgFilterMin -> Specifies the value of the first message in the range of
                   messages to be examined.
  wMsgFilterMax -> Specifies the value of the last message in the range of
                   messages to be examined.
  wMsgTerminate -> Specifies the value of the message which terminates the
                   message processing. The message specified here will not
                   be processed.

 Results:
  If the message specified by wMsgTerminate or the WM_QUIT message has
  been received, the return value is FALSE, otherwise the return value
  is TRUE.

 Remarks:
  The function processes only messages associated with the dialog box or any
  of its children as specified by the IsChild function, and within the range
  of message values given by the wMsgFilterMin and wMsgFilterMax parameters.
  If fAllWindows is TRUE, the function processes messages for any window that
  belongs to the current thread making the call.
  If wMsgFilterMin and wMsgFilterMax are both zero, the function processes all
  available messages (that is, no range filtering is performed). Note that the
  WM_QUIT message will always terminate the message processing. The function
  does not process messages for windows that belong to other threads.
*****************************************************************************/
BOOL CDialogWnd::ProcessMessageA(BOOL fAllWindows, UINT wMsgFilterMin,
                                 UINT wMsgFilterMax, UINT wMsgTerminate)
{
  HWND hwnd;
  MSG  sMsg;
  BOOL fRes;

  hwnd = fAllWindows ? NULL : m_hwndDlg;
  fRes = TRUE;

  if (::PeekMessageA(&sMsg, hwnd, wMsgFilterMin, wMsgFilterMax, PM_REMOVE))
  {
    if ((sMsg.message == WM_QUIT) || (sMsg.message == wMsgTerminate))
    {
      fRes = FALSE;
    }
    else
    {
      if (!::IsDialogMessageA(m_hwndDlg, &sMsg))
      {
        ::TranslateMessage(&sMsg);
        ::DispatchMessageA(&sMsg);
      }
    }
  }

  return( fRes );
}
/*****************************************************************************
 End of CDialogWnd::ProcessMessageA
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::ProcessMessageW

 Description:
  This function checks the thread message queue for a posted message
  intended either for the dialog box, or any other window, and process
  a single message (if any exist).

 Arguments:
  fAllWindows   -> If set to TRUE the function process all messages for
                   all windows of the thread. If set to FALSE the function
                   processes only those messages intended for the dialog box.
  wMsgFilterMin -> Specifies the value of the first message in the range of
                   messages to be examined.
  wMsgFilterMax -> Specifies the value of the last message in the range of
                   messages to be examined.
  wMsgTerminate -> Specifies the value of the message which terminates the
                   message processing. The message specified here will not
                   be processed.

 Results:
  If the message specified by wMsgTerminate or the WM_QUIT message has
  been received, the return value is FALSE, otherwise the return value
  is TRUE.

 Remarks:
  The function processes only messages associated with the dialog box or any
  of its children as specified by the IsChild function, and within the range
  of message values given by the wMsgFilterMin and wMsgFilterMax parameters.
  If fAllWindows is TRUE, the function processes messages for any window that
  belongs to the current thread making the call.
  If wMsgFilterMin and wMsgFilterMax are both zero, the function processes all
  available messages (that is, no range filtering is performed). Note that the
  WM_QUIT message will always terminate the message processing. The function
  does not process messages for windows that belong to other threads.
*****************************************************************************/
BOOL CDialogWnd::ProcessMessageW(BOOL fAllWindows, UINT wMsgFilterMin,
                                 UINT wMsgFilterMax, UINT wMsgTerminate)
{
  HWND hwnd;
  MSG  sMsg;
  BOOL fRes;

  hwnd = fAllWindows ? NULL : m_hwndDlg;
  fRes = TRUE;

  if (::PeekMessageW(&sMsg, hwnd, wMsgFilterMin, wMsgFilterMax, PM_REMOVE))
  {
    if ((sMsg.message == WM_QUIT) || (sMsg.message == wMsgTerminate))
    {
      fRes = FALSE;
    }
    else
    {
      if (!::IsDialogMessageW(m_hwndDlg, &sMsg))
      {
        ::TranslateMessage(&sMsg);
        ::DispatchMessageW(&sMsg);
      }
    }
  }

  return( fRes );
}
/*****************************************************************************
 End of CDialogWnd::ProcessMessageW
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::ProcessMessagesA

 Description:
  This function checks the thread message queue for posted messages
  intended either for the dialog box, or any other window, and
  processes the messages (if any exist).

 Arguments:
  fAllWindows   -> If set to TRUE the function process all messages for
                   all windows of the thread. If set to FALSE the function
                   processes only those messages intended for the dialog box.
  wMsgFilterMin -> Specifies the value of the first message in the range of
                   messages to be examined.
  wMsgFilterMax -> Specifies the value of the last message in the range of
                   messages to be examined.
  wMsgTerminate -> Specifies the value of the message which terminates the
                   message processing. The message specified here will not
                   be processed.

 Results:
  If the message specified by wMsgTerminate or the WM_QUIT message has
  been received, the return value is FALSE, otherwise the return value
  is TRUE.

 Remarks:
  The function processes only messages associated with the dialog box or any
  of its children as specified by the IsChild function, and within the range
  of message values given by the wMsgFilterMin and wMsgFilterMax parameters.
  If fAllWindows is TRUE, the function processes messages for any window that
  belongs to the current thread making the call.
  If wMsgFilterMin and wMsgFilterMax are both zero, the function processes all
  available messages (that is, no range filtering is performed). Note that the
  WM_QUIT message will always terminate the message processing. The function
  does not process messages for windows that belong to other threads.
*****************************************************************************/
BOOL CDialogWnd::ProcessMessagesA(BOOL fAllWindows, UINT wMsgFilterMin,
                                  UINT wMsgFilterMax, UINT wMsgTerminate)
{
  HWND hwnd;
  MSG  sMsg;
  BOOL fRes;

  hwnd = fAllWindows ? NULL : m_hwndDlg;
  fRes = TRUE;

  while (::PeekMessageA(&sMsg, hwnd, wMsgFilterMin, wMsgFilterMax, PM_REMOVE))
  {
    if ((sMsg.message == WM_QUIT) || (sMsg.message == wMsgTerminate))
    {
      fRes = FALSE;
      break;
    }
    else
    {
      if (!::IsDialogMessageA(m_hwndDlg, &sMsg))
      {
        ::TranslateMessage(&sMsg);
        ::DispatchMessageA(&sMsg);
      }
    }
  }

  return( fRes );
}
/*****************************************************************************
 End of CDialogWnd::ProcessMessagesA
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::ProcessMessagesW

 Description:
  This function checks the thread message queue for posted messages
  intended either for the dialog box, or any other window, and
  processes the messages (if any exist).

 Arguments:
  fAllWindows   -> If set to TRUE the function process all messages for
                   all windows of the thread. If set to FALSE the function
                   processes only those messages intended for the dialog box.
  wMsgFilterMin -> Specifies the value of the first message in the range of
                   messages to be examined.
  wMsgFilterMax -> Specifies the value of the last message in the range of
                   messages to be examined.
  wMsgTerminate -> Specifies the value of the message which terminates the
                   message processing. The message specified here will not
                   be processed.

 Results:
  If the message specified by wMsgTerminate or the WM_QUIT message has
  been received, the return value is FALSE, otherwise the return value
  is TRUE.

 Remarks:
  The function processes only messages associated with the dialog box or any
  of its children as specified by the IsChild function, and within the range
  of message values given by the wMsgFilterMin and wMsgFilterMax parameters.
  If fAllWindows is TRUE, the function processes messages for any window that
  belongs to the current thread making the call.
  If wMsgFilterMin and wMsgFilterMax are both zero, the function processes all
  available messages (that is, no range filtering is performed). Note that the
  WM_QUIT message will always terminate the message processing. The function
  does not process messages for windows that belong to other threads.
*****************************************************************************/
BOOL CDialogWnd::ProcessMessagesW(BOOL fAllWindows, UINT wMsgFilterMin,
                                  UINT wMsgFilterMax, UINT wMsgTerminate)
{
  HWND hwnd;
  MSG  sMsg;
  BOOL fRes;

  hwnd = fAllWindows ? NULL : m_hwndDlg;
  fRes = TRUE;

  while (::PeekMessageW(&sMsg, hwnd, wMsgFilterMin, wMsgFilterMax, PM_REMOVE))
  {
    if ((sMsg.message == WM_QUIT) || (sMsg.message == wMsgTerminate))
    {
      fRes = FALSE;
      break;
    }
    else
    {
      if (!::IsDialogMessageW(m_hwndDlg, &sMsg))
      {
        ::TranslateMessage(&sMsg);
        ::DispatchMessageW(&sMsg);
      }
    }
  }

  return( fRes );
}
/*****************************************************************************
 End of CDialogWnd::ProcessMessagesW
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::DisplayErrorMessageA

 Description:
  This function displays an error message box for the specified error.

 Arguments:
  pszError -> Points to the error message to be displayed. If this
              parameter is NULL, no message box is displayed by the
              function.

 Results:
  none
*****************************************************************************/
void CDialogWnd::DisplayErrorMessageA(PCSTR pszError)
{
  if (pszError != NULL)
    ::MessageBoxA(m_hwndDlg, pszError, "Error", MB_OK|MB_ICONERROR);
}
/*****************************************************************************
 End of CDialogWnd::DisplayErrorMessageA
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::DisplayErrorMessageW

 Description:
  This function displays an error message box for the specified error.

 Arguments:
  pszError -> Points to the error message to be displayed. If this
              parameter is NULL, no message box is displayed by the
              function.

 Results:
  none
*****************************************************************************/
void CDialogWnd::DisplayErrorMessageW(PCWSTR pszError)
{
  if (pszError != NULL)
    ::MessageBoxW(m_hwndDlg, pszError, L"Error", MB_OK|MB_ICONERROR);
}
/*****************************************************************************
 End of CDialogWnd::DisplayErrorMessageW
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::DisplayErrorCode

 Description:
  This function displays an error message box for the specified error.

 Arguments:
  hResult -> Error code or -1 to retrieve the error code by a call to
             GetLastError. If this parameter is set to NO_ERROR, no
             message box is displayed by the function

 Results:
  none
*****************************************************************************/
void CDialogWnd::DisplayErrorCode(HRESULT hResult)
{
  static PWCHAR szErrMsg[] =
  {
    L"This operation returned because the timeout period expired.",
    NULL
  };

  WCHAR szError[1024];
  DWORD len;

  if (hResult != NO_ERROR)
  {
    if (hResult == -1)
      hResult = GetLastError();

    szError[0] = 0;

    len = ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
                           FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                           hResult, 0, szError, _countof(szError), NULL);
    if (len == 0)
    {
      switch (hResult)
      {
        case ERROR_TIMEOUT:
          wcscpy_s(szError, szErrMsg[0]);
          break;

        default:
          swprintf_s(szError, _countof(szError),
            L"Error code: 0x%08lX (%ld)", (ULONG) hResult, hResult);
          break;
      }
    }

    DisplayErrorMessageW(szError);
  }
}
/*****************************************************************************
 End of CDialogWnd::DisplayErrorCode
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::CmdProc

 Description:
  This function is an application-defined callback function which is called
  on every WM_COMMAND message sent to the modal dialog box.

 Parameter:
  wId   -> Specifies the identifier of the menu item, control, or accelerator.
  wCode -> Specifies the notification code if the message is from a control.
           If the message is from an accelerator, this parameter is 1. If the
           message is from a menu, this parameter is 0.
  hCtrl -> Handle to the control sending the message if the message is from
           a control. Otherwise, this parameter is NULL. 

 Results:
  If the function processes the message it returns TRUE, otherwise the
  function returns FALSE.
*****************************************************************************/
BOOL CDialogWnd::CmdProc(WORD wId, WORD wCode, HWND hCtrl)
{
  BOOL fResult;

  if (m_fIsModal == TRUE)
  {
    //
    // default WM_COMMAND handler for modal dialog boxes
    //
    switch(wId)
    {
      case IDOK    : // fall through
      case IDCANCEL: // fall through
      case IDABORT : // fall through
      case IDRETRY : // fall through
      case IDIGNORE: // fall through
      case IDYES   : // fall through
      case IDNO    :
        EndDialog(wId);
        fResult = TRUE;
        break;

      default:
        fResult = FALSE;
        break;
    }
  }
  else
  {
    //
    // default WM_COMMAND handler for modeless dialog boxes
    //
    fResult = FALSE;
  }

  return( fResult );
}
/*****************************************************************************
 End of CDialogWnd::CmdProc
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::DlgProc

 Description:
  This function is an application-defined callback function used with the
  DialogBox function. It processes messages sent to a modal dialog box.

 Arguments:
  uMsg   -> Specifies the message.
  wParam -> Specifies additional message information. The contents of this
            parameter depend on the value of the uMsg parameter. 
  lParam -> Specifies additional message information. The contents of this
            parameter depend on the value of the uMsg parameter. 

 Results :
  Typically, the dialog box procedure should return TRUE if it processed the
  message, and FALSE if it did not. If the dialog box procedure returns FALSE,
  the dialog manager performs the default dialog operation in response to the
  message.
*****************************************************************************/
BOOL CDialogWnd::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  BOOL fResult;

  switch (uMsg)
  {
    case WM_COMMAND:
      fResult = CmdProc(LOWORD(wParam), HIWORD(wParam), (HWND) lParam);
      break;

    default:
      fResult = FALSE;
  }

  return( fResult );
}
/*****************************************************************************
 End of CDialogWnd::DlgProc
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::StdDlgProc

 Description:
  The function is an class-defined callback function used with the
  DialogBox functions. It processes messages sent to a modal dialog
  box. The DLGPROC type defines a pointer to this callback function.

 Arguments:
  hwnd   -> Handle to the dialog box window.
  uMsg   -> Specifies the message.
  wParam -> Specifies additional message information. The contents of
            this parameter depend on the value of the uMsg parameter. 
  lParam -> Specifies additional message information. The contents of
            this parameter depend on the value of the uMsg parameter. 

 Results:
  Typically, the dialog box procedure should return TRUE if it processed the
  message, and FALSE if it did not. If the dialog box procedure returns FALSE,
  the dialog manager performs the default dialog operation in response to the
  message.
*****************************************************************************/
INT_PTR CALLBACK CDialogWnd::StdDlgProc(HWND hwnd, UINT uMsg,
                                        WPARAM wParam, LPARAM lParam)
{
  BOOL     fRes = FALSE;
  PDLGPARA pPar = (PDLGPARA) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);

  if ((pPar != NULL) && (pPar->pdlg != NULL))
  {
    pPar->pdlg->m_hwndDlg = hwnd;
    fRes = pPar->pdlg->DlgProc(uMsg, wParam, lParam);
    if (uMsg == WM_DESTROY)
      pPar->pdlg->m_hwndDlg = NULL;
  }

  return( fRes );
}
/*****************************************************************************
 End of CDialogWnd::StdDlgProc
*****************************************************************************/


/*****************************************************************************
 Function:
  CDialogWnd::DlgWndProc

 Description:
  This function is the class-defined function that processes messages
  sent to a dialog window.

 Parameter:
  hwnd   -> Handle to the window.
  uMsg   -> Specifies the message.
  wParam -> Specifies additional message information. The contents of this
            parameter depend on the value of the uMsg parameter. 
  lParam -> Specifies additional message information. The contents of this
            parameter depend on the value of the uMsg parameter.

 Results:
  The return value is the result of the message processing and depends
  on the message sent.
*****************************************************************************/
LRESULT CALLBACK CDialogWnd::DlgWndProc(HWND hwnd, UINT uMsg,
                                        WPARAM wParam, LPARAM lParam)
{
  LRESULT lRes =  -1;

  switch (uMsg)
  {
    //--------------------------------------------------------------------
    case WM_CREATE:
    //--------------------------------------------------------------------
    {
      lRes = ::DefDlgProc(hwnd, uMsg, wParam, lParam);

      if (lRes == 0)
      {
        PDLGPARA pPar = (PDLGPARA) ::LocalAlloc(LPTR, sizeof(DLGPARA));

        if (pPar != NULL)
        {
          SetLastError(0);
          lRes = ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) pPar);
          lRes = (lRes == 0) ? GetLastError() : 0;

          if (lRes == 0)
          {
            RECT rect;
            if (::GetClientRect(hwnd, &rect))
            {
              pPar->minx = 1;
              pPar->miny = 1;
              pPar->dlgw = rect.right;
              pPar->dlgh = rect.bottom;
              pPar->winw = rect.right;
              pPar->winh = rect.bottom;
              pPar->oldx = 0;
              pPar->oldy = 0;
            }
            else
            {
              lRes = GetLastError();
            }
          }
        }
        else
        {
          lRes = -1; // out of memory
        }
      }
    } break;

    //--------------------------------------------------------------------
    case WM_DESTROY:
    //--------------------------------------------------------------------
    {
      lRes = ::DefDlgProc(hwnd, uMsg, wParam, lParam);

      HLOCAL hMem = (HLOCAL) ::SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);

      if (hMem != NULL)
        LocalFree(hMem);
    } break;

    //--------------------------------------------------------------------
    case WM_INITDIALOG:
    //--------------------------------------------------------------------
    {
      PDLGPARA pPar = (PDLGPARA) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);

      if (pPar != NULL)
      {
        DWORD Dims = GetSmallestDlgCtrl(hwnd);

        pPar->minx = LOWORD(Dims);
        pPar->miny = HIWORD(Dims);

        pPar->pdlg = reinterpret_cast<CDialogWnd*>(lParam);

        if (pPar->pdlg != NULL)
        {
          pPar->pdlg->m_hwndDlg = hwnd;
          lParam = pPar->pdlg->m_lInitPar;
        }
      }

      lRes = ::DefDlgProc(hwnd, uMsg, wParam, lParam);
    } break;

    //--------------------------------------------------------------------
    case WM_SIZE:
    //--------------------------------------------------------------------
    {
      if ((wParam == SIZE_RESTORED) || (wParam == SIZE_MAXIMIZED))
      {
        PDLGPARA pPar = (PDLGPARA) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);

        if (pPar != NULL)
        {
          SCROLLINFO six;  // horizontal scroll bar info
          SCROLLINFO siy;  // vertical scroll bar info
          int        scrx; // current horizontal scroll range
          int        scry; // current vertical scroll range
          int        curx; // current horizontal scroll position
          int        cury; // current vertical scroll position

          //
          // get current position of the horizontal scroll bar
          //
          six.cbSize = sizeof(SCROLLINFO);
          six.fMask  = SIF_ALL;

          if (!::GetScrollInfo(hwnd, SB_HORZ, &six))
            six.nPos = 0;

          //
          // get current position of the vertical scroll bar
          //
          siy.cbSize = sizeof(SCROLLINFO);
          siy.fMask  = SIF_ALL;

          if (!::GetScrollInfo(hwnd, SB_VERT, &siy))
            siy.nPos = 0;

          //
          // store new size of client area
          //
          pPar->winw = (int) LOWORD(lParam);
          pPar->winh = (int) HIWORD(lParam);

          //
          // compute new scroll range
          //
          scrx = max(0, pPar->dlgw - pPar->winw);
          scry = max(0, pPar->dlgh - pPar->winh);

          //
          // get current scroll position
          //
          curx = min(six.nPos, scrx);
          cury = min(siy.nPos, scry);

          //
          // set position of the horizontal scroll bar
          //
          six.fMask |= SIF_DISABLENOSCROLL;
          six.nMin   = 0;
          six.nMax   = pPar->dlgw - 1;
          six.nPage  = pPar->winw;
          six.nPos   = curx;
          ::SetScrollInfo(hwnd, SB_HORZ, &six, TRUE);

          //
          // set position of the vertical scroll bar
          //
          siy.fMask |= SIF_DISABLENOSCROLL;
          siy.nMin   = 0;
          siy.nMax   = pPar->dlgh - 1;
          siy.nPage  = pPar->winh;
          siy.nPos   = cury;
          ::SetScrollInfo(hwnd, SB_VERT, &siy, TRUE);

          MoveDlgCtrls(hwnd, pPar->oldx - curx, pPar->oldy - cury);

          //
          // store current scroll position
          //
          pPar->oldx = curx;
          pPar->oldy = cury;
        }
      }

      lRes  = ::DefDlgProc(hwnd, uMsg, wParam, lParam);
    } break;

    //--------------------------------------------------------------------
    case WM_HSCROLL:
    //--------------------------------------------------------------------
    {
      if (lParam == NULL)
      {
        //
        // message sent by a standard scroll bar
        //
        PDLGPARA pPar = (PDLGPARA) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);

        if (pPar != NULL)
        {
          SCROLLINFO si;
          int        pgsz;
          int        xmin;
          int        xmax;
          int        xold;
          int        x, y;

          si.cbSize = sizeof(SCROLLINFO);
          si.fMask  = SIF_ALL;
          ::GetScrollInfo(hwnd, SB_HORZ, &si);

          pgsz = (int) si.nPage;
          xmin = si.nMin;
          xmax = max(0, pPar->dlgw - pgsz);
          xold = si.nPos;

          switch (LOWORD(wParam))
          {
            case SB_TOP          : x = xmin;                         break;
            case SB_BOTTOM       : x = xmax;                         break;
            case SB_PAGEUP       : x = max(xmin, xold - pgsz);       break;
            case SB_PAGEDOWN     : x = min(xmax, xold + pgsz);       break;
            case SB_LINEUP       : x = max(xmin, xold - pPar->minx); break;
            case SB_LINEDOWN     : x = min(xmax, xold + pPar->minx); break;
            case SB_THUMBPOSITION: x = HIWORD(wParam);               break;
            case SB_THUMBTRACK   : x = HIWORD(wParam);               break;
            case SB_ENDSCROLL    :
            default              : x = xold;
          }

          if (x != xold)
          {
            si.fMask = SIF_POS;
            si.nPos  = x;
            ::SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
            ::GetScrollInfo(hwnd, SB_VERT, &si);
            y = si.nPos;
            MoveDlgCtrls(hwnd, pPar->oldx - x, pPar->oldy - y);
            pPar->oldx = x;
            pPar->oldy = y;
          }

          lRes = 0;
        }
        else
        {
          lRes = ::DefDlgProc(hwnd, uMsg, wParam, lParam);
        }
      }
      else
      {
        //
        // message sent by a scroll bar control
        //
        lRes = ::DefDlgProc(hwnd, uMsg, wParam, lParam);
      }
    } break;

    //--------------------------------------------------------------------
    case WM_VSCROLL:
    //--------------------------------------------------------------------
    {
      if (lParam == NULL)
      {
        //
        // message sent by a standard scroll bar
        //
        PDLGPARA pPar = (PDLGPARA) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);

        if (pPar != NULL)
        {
          SCROLLINFO si;
          int        pgsz;
          int        ymin;
          int        ymax;
          int        yold;
          int        x, y;

          si.cbSize = sizeof(SCROLLINFO);
          si.fMask  = SIF_ALL;
          ::GetScrollInfo(hwnd, SB_VERT, &si);

          pgsz = (int) si.nPage;
          ymin = si.nMin;
          ymax = max(0, pPar->dlgh - pgsz);
          yold = si.nPos;

          switch (LOWORD(wParam))
          {
            case SB_TOP          : y = ymin;                         break;
            case SB_BOTTOM       : y = ymax;                         break;
            case SB_PAGEUP       : y = max(ymin, yold - pgsz);       break;
            case SB_PAGEDOWN     : y = min(ymax, yold + pgsz);       break;
            case SB_LINEUP       : y = max(ymin, yold - pPar->miny); break;
            case SB_LINEDOWN     : y = min(ymax, yold + pPar->miny); break;
            case SB_THUMBPOSITION: y = HIWORD(wParam);               break;
            case SB_THUMBTRACK   : y = HIWORD(wParam);               break;
            case SB_ENDSCROLL    :
            default              : y = yold;
          }

          if (y != yold)
          {
            si.fMask = SIF_POS;
            si.nPos  = y;
            ::SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            ::GetScrollInfo(hwnd, SB_HORZ, &si);
            x = si.nPos;
            MoveDlgCtrls(hwnd, pPar->oldx - x, pPar->oldy - y);
            pPar->oldx = x;
            pPar->oldy = y;
          }

          lRes = 0;
        }
        else
        {
          lRes = ::DefDlgProc(hwnd, uMsg, wParam, lParam);
        }
      }
      else
      {
        //
        // message sent by a scroll bar control
        //
        lRes = ::DefDlgProc(hwnd, uMsg, wParam, lParam);
      }
    } break;

    //--------------------------------------------------------------------
    default:
    //--------------------------------------------------------------------
    {
      lRes = ::DefDlgProc(hwnd, uMsg, wParam, lParam);
    }
  }

  return( lRes );
}
/*****************************************************************************
 End of CDialogWnd::DlgWndProc
*****************************************************************************/

#ifdef _MSC_VER
#pragma warning(default:4100)     // unreferenced formal parameter
#endif

