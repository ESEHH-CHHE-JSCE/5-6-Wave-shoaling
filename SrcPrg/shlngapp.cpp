//----------------------------------------------------------------------------
//  �󐅕ό`�\�� �i���������W���W)
//        1998/06/09 by tonto
//  This Progaram is coded based on
//         ObjectWindows - (C) Copyright 1991, 1994 by Borland International
//         �`���[�g���A�� �A�v���P�[�V���� -- step10.cpp
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/applicat.h>
#include <owl/decframe.h>
#include <owl/dc.h>
#include <owl/inputdia.h>
#include <owl/opensave.h>
#include <owl/controlb.h>
#include <owl/buttonga.h>
#include <owl/statusba.h>
#include <owl/gdiobjec.h>
#include <owl/chooseco.h>
#include <owl/scroller.h>
#include <owl/color.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dir.h>
#include <classlib/arrays.h>
#include "shlngapp.rc"
#include "shoaling.h"


char HelpFileName[MAXPATH];

// ----  for TLine Class  ----

bool TLine::Draw(TDC &dc) const
{
  // ���̐��̃y���̒l���Cdc �ɐݒ肷��
  TPen pen(Color, PenSize);
  dc.SelectObject(pen);

  // �� i �̊e�_�̍��W�ɂ��āC��������
  TPointsIterator j(*this);
  bool first = true;

  while (j) {
    TPoint p = j++;

    if (!first)
      dc.LineTo(p);
    else {
      dc.MoveTo(p);
      first = false;
    }
  }
  dc.RestorePen();
  return true;
}


//**  for TGText Class

TGText::TGText ( TPoint pos, uint align, TColor color, char *text )
     :Pos(pos), aAlign(align), Color(color)
  {
    tText = text ;
  }

TGText::~TGText ()
  {
//    free ( tText ) ;
  }


bool TGText::Draw(TDC &dc) const
{
  dc.SetTextAlign (aAlign) ;
  dc.SetTextColor (Color) ;
  dc.TextOut ( Pos, tText ) ;
  return ( true ) ;
}


class TDrawWindow : public TWindow {
  public:
    TDrawWindow(TWindow *parent = 0);
   ~TDrawWindow() {delete DragDC; delete Lines; delete Texts; delete FileData;}

  protected:
    TDC *DragDC;
    TLines *Lines;
    TGTexts *Texts;
    TOpenSaveDialog::TData
              *FileData;
    bool IsDirty, IsNewFile, IsNewData;
    TRect clientRect ;

// TWindow �̃����o�[�֐����I�[�o�[���C�h����
    bool CanClose();

    // ���b�Z�[�W�����֐�
    void Paint(TDC&, bool, TRect&);
    void AdjustScroller ( void ) ;
    bool IdleAction ( long idleCount ) ;
    void CmFileSave();
    void CmSetSlope() ;
    void CmSetWaveHeight() ;
    void CmSetWavePeriod() ;
    void CmAbout();
    void CmSHelp();
    void SaveFile();

    // User Functions and Variables
    void SetDrawingData ( void ) ;
    float H0, T0, slope ;


    DECLARE_RESPONSE_TABLE(TDrawWindow);
};

DEFINE_RESPONSE_TABLE1(TDrawWindow, TWindow)
  EV_COMMAND(CM_FILESAVE, CmFileSave),
  EV_COMMAND(CM_SLOPE, CmSetSlope),
  EV_COMMAND(CM_HEIGHT, CmSetWaveHeight),
  EV_COMMAND(CM_PERIOD, CmSetWavePeriod),
  EV_COMMAND(CM_ABOUT, CmAbout),
  EV_COMMAND(CM_SHELP, CmSHelp),
END_RESPONSE_TABLE;

TDrawWindow::TDrawWindow(TWindow *parent)
{
  Init(parent, 0, 0);
  DragDC = 0;
  Lines = new TLines(5, 0, 5);
  Texts = new TGTexts(5, 0, 5 ) ;
  IsNewFile = true;
  IsDirty = false;
  FileData = new TOpenSaveDialog::TData(OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
                                        "�ް�̧�� (*.DAT)|*.DAT|"
                                        "���ׂĂ�̧�� (*.*)|*.*|",
                                         0, "", "DAT");

  Attr.Style |= WS_BORDER | WS_CAPTION | WS_VSCROLL | WS_HSCROLL | WS_SYSMENU ;
  Attr.ExStyle |= WS_EX_CLIENTEDGE ;
  Scroller = new TScroller ( this, 1, 1, 200, 200 ) ;
//  Scroller->SetRange ( 600, 600 ) ;

//----  Initialization for Wave Information  ----
  IsNewData = true ;
  H0 = 1.0 ;
  T0 = 10.0 ;
  slope = 100.0 ;
}


bool TDrawWindow::CanClose()
{
  if (IsDirty)
    switch(MessageBox("�v�Z���ʂ�ۑ����܂���?", "�v�Z���ʂ��ۑ�����Ă��܂���",
                      MB_YESNOCANCEL | MB_ICONQUESTION)) {
      case IDCANCEL:
        // Cancel ��I������ƃE�B���h�E�����̂��~�߂� -- false ��Ԃ�
        return false;

      case IDYES:
        // Yes ��I������ƕ`����e��ۑ�����
         CmFileSave();
    }
  return true;
}

void TDrawWindow::CmSetSlope()
  {
  char inputText[16] ;
  float  invSlope = slope ;

  sprintf ( inputText, "%g", invSlope ) ;

  if ((TInputDialog(this, "�C����z",
                        "�C����z�̋t�������(��:1/100���z,'100'�����):",
                        inputText,
                        sizeof(inputText))).Execute() == IDOK) {
    invSlope = atof(inputText);
    if ( invSlope < 1.0 )
        MessageBox ( "�C����z(�̋t��)�ɂ�1�ȏ�̒l����͂��Ă��������D", "�f�[�^�G���[", MB_OK | MB_ICONEXCLAMATION ) ;
      else
        {
           slope = invSlope ;

          IsNewData = true ;
          Invalidate() ;
        }
    }
  }


void TDrawWindow::CmSetWaveHeight()
  {
  char inputText[16] ;
  float  WaveHeight = H0 ;

  sprintf ( inputText, "%g", WaveHeight ) ;

  if ((TInputDialog(this, "���g�g��",
                        "���g�g��Ho����� (�P��:m):",
                        inputText,
                        sizeof(inputText))).Execute() == IDOK) {
    WaveHeight = atof(inputText);
    if ( WaveHeight <= 0.0 )
        MessageBox ( "���g�g���ɂ͐��̒l����͂��Ă��������D", "�f�[�^�G���[", MB_OK | MB_ICONEXCLAMATION ) ;
      else
        {
          H0 = WaveHeight ;

          IsNewData = true ;
          Invalidate() ;
        }
    }
  }


void TDrawWindow::CmSetWavePeriod()
  {
  char inputText[16] ;
  float  WavePeriod = T0 ;

  sprintf ( inputText, "%g", WavePeriod ) ;

  if ((TInputDialog(this, "����",
                        "���g�̎���T����� (�P��:s):",
                        inputText,
                        sizeof(inputText))).Execute() == IDOK) {
    WavePeriod = atof(inputText);
    if ( WavePeriod <= 0.0 )
        MessageBox ( "�����ɂ͐��̒l����͂��Ă��������D", "�f�[�^�G���[", MB_OK | MB_ICONEXCLAMATION ) ;
      else
        {
          T0 = WavePeriod ;

          IsNewData = true ;
          Invalidate() ;
        }
    }
  }


void TDrawWindow::Paint(TDC& dc, bool, TRect&)
{
  if ( IsNewData )
    {
      delete Lines ;
      delete Texts ;
      TShoalingGraData Graph ;
      TGraData *GraData = Graph.SetGraphicData ( H0, T0, slope ) ;
      Lines = GraData->Lines ;
      Texts = GraData->Texts ;
      IsNewData = false ;
      IsDirty = true ;
    }

  // ���I�u�W�F�N�g�̔z��ɂ��āC��������
  TLinesIterator i(*Lines);

  while (i)
    i++.Draw(dc);

  TGTextsIterator t(*Texts);

  while (t)
    t++.Draw(dc);

  AdjustScroller () ;
}


void  TDrawWindow::AdjustScroller ( void )
  {
    #define  xViewRange  730
    #define  yViewRange  560

    clientRect = GetClientRect() ;
    Scroller->SetRange ( Max ( xViewRange - clientRect.Width(), 0 ),
                         Max ( yViewRange - clientRect.Height(), 0 ) ) ;

    #undef  xViewRange
    #undef  yViewRange
  }


bool  TDrawWindow::IdleAction ( long idleCount )
  {
    TRect cRect = GetClientRect () ;
    if (  ( clientRect.Width() != cRect.Width() )
        || ( clientRect.Height() != cRect.Height() ) )
      Invalidate() ;
    return (true) ;
  }


void TDrawWindow::CmFileSave()
{
  if ((TFileSaveDialog(this, *FileData)).Execute() == IDOK)
    SaveFile();
}

void TDrawWindow::CmAbout()
{
  TDialog(this, IDD_ABOUT).Execute();
}

void TDrawWindow::CmSHelp()
{
  WinHelp ( HelpFileName, HELP_FORCEFILE, 0 ) ;
}

void TDrawWindow::SaveFile()
{
  FILE *fp ;

  if ( ( fp = fopen ( FileData->FileName, "w" ) ) == NULL )
      MessageBox("̧�ق��J���܂���", "̧�ٴװ", MB_OK | MB_ICONEXCLAMATION);
    else
      {
        TShoalingGraData Graph ;
        Graph.SetSaveData ( fp, H0, T0, slope ) ;
        fclose ( fp ) ;

        IsNewFile = IsDirty = false;
      }
}

class TDrawApp : public TApplication {
  public:
    TDrawApp() : TApplication() {}

    void InitMainWindow();
};

void TDrawApp::InitMainWindow()
{
  // �������t���[���E�B���h�E���R���X�g���N�g����
  TDecoratedFrame* frame = new TDecoratedFrame(0, "���������W���W  �󐅕ό`", new TDrawWindow, true);

  // �X�e�[�^�X�o�[���R���X�g���N�g����
  TStatusBar* sb = new TStatusBar(frame, TGadget::Recessed,
                                  TStatusBar::CapsLock        |
                                  TStatusBar::NumLock         |
                                  TStatusBar::ScrollLock);
 
  // �R���g���[���o�[���R���X�g���N�g����
  TControlBar *cb = new TControlBar(frame);
  cb->Insert(*new TButtonGadget(CM_FILESAVE, CM_FILESAVE, TButtonGadget::Command));
  cb->Insert(*new TSeparatorGadget);
  cb->Insert(*new TButtonGadget(CM_SLOPE, CM_SLOPE, TButtonGadget::Command));
  cb->Insert(*new TButtonGadget(CM_HEIGHT, CM_HEIGHT, TButtonGadget::Command));
  cb->Insert(*new TButtonGadget(CM_PERIOD, CM_PERIOD, TButtonGadget::Command));
  cb->Insert(*new TSeparatorGadget);
  cb->Insert(*new TButtonGadget(CM_SHELP, CM_SHELP, TButtonGadget::Command));

  // �L���v�V�����ƃq���g���[�h��ݒ肷��
  //
  cb->SetCaption("°��ް");
  cb->SetHintMode(TGadgetWindow::EnterHints);

  // �t���[���̒��ɃX�e�[�^�X�o�[�ƃR���g���[���o�[��}������
  frame->Insert(*sb, TDecoratedFrame::Bottom);
  frame->Insert(*cb, TDecoratedFrame::Top);

  // ���C���E�B���h�E�ƃE�B���h�E�̃��j���[��ݒ肷��
  SetMainWindow(frame);
  GetMainWindow()->AssignMenu("COMMANDS");
}

void  sethelpname ( char *exename )
  {
    char drive[MAXDRIVE];
    char dir[MAXDIR];
    char file[MAXFILE];
    char ext[MAXEXT];

    fnsplit(exename,drive,dir,file,ext);
    fnmerge(HelpFileName,drive,dir,file,".hlp" );
  }


int OwlMain(int /*argc*/, char* argv [])
{
  sethelpname ( argv[0] ) ;

  return TDrawApp().Run();
}

