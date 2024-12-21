//----------------------------------------------------------------------------
// ObjectWindows - (C) Copyright 1991, 1994 by Borland International
//   �`���[�g���A�� �A�v���P�[�V���� -- step10.cpp
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
#include <stdlib.h>
#include <string.h>
#include <classlib/arrays.h>
#include "shlngapp.rc"

typedef TArray<TPoint> TPoints;
typedef TArrayIterator<TPoint> TPointsIterator;

class TLine : public TPoints {
  public:
    // �F�ƃy���̑������w��ł���R���X�g���N�^
    // �܂��C�f�t�H���g�̃R���X�g���N�^�Ƃ��Ă����p�ł���
    TLine(const TColor &color = TColor(0), int penSize = 1)
      : TPoints(10, 0, 10), PenSize(penSize), Color(color) {}

    // �y���̑����������ˁC�ύX����֐�
    int QueryPenSize() { return PenSize; }
    TColor& QueryColor() { return Color; }
    void SetPen(TColor &newColor, int penSize = 0);
    void SetPen(int penSize);

    // TLine �́C���ꎩ�g��`�悷��B���ׂĂ��܂��������Ȃ�Ctrue ��Ԃ�
    virtual bool Draw(TDC &) const;

    // == ���Z�q�́C�g�p����Ă��Ȃ��Ă��R���e�i�N���X�p�ɒ�`���Ȃ���΂Ȃ�Ȃ�
    bool operator ==(const TLine& other) const { return &other == this; }
    friend ostream& operator <<(ostream& os, const TLine& line);
    friend istream& operator >>(istream& is, TLine& line);

  protected:
    int PenSize;
    TColor Color;
};

void TLine::SetPen(int penSize)
{
  if (penSize < 1)
    PenSize = 1;
  else
    PenSize = penSize;
}

void TLine::SetPen(TColor &newColor, int penSize)
{
  // penSize ���f�t�H���g�l (0) �łȂ��Ȃ�CPenSize �ɐV�������̑�����ݒ肷��
  if (penSize)
    PenSize = penSize;

  Color = newColor;
}

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

typedef TArray<TLine> TLines;
typedef TArrayIterator<TLine> TLinesIterator;

class TDrawWindow : public TWindow {
  public:
    TDrawWindow(TWindow *parent = 0);
   ~TDrawWindow() {delete DragDC; delete Line; delete Lines; delete FileData;}

  protected:
    TDC *DragDC;
    TPen *Pen;
    TLines *Lines;
    TLine *Line; // �����P�{�����ꎞ�I�Ɋi�[����B����͌�ŁCLINES �Ɋi�[���Ȃ���
    TOpenSaveDialog::TData
              *FileData;
    bool IsDirty, IsNewFile;

    void GetPenSize(); // GetPenSize �͏�� Line->SetPen() ���Ăяo��

    // TWindow �̃����o�[�֐����I�[�o�[���C�h����
    bool CanClose();

    // ���b�Z�[�W�����֐�
    void EvLButtonDown(uint, TPoint&);
    void EvRButtonDown(uint, TPoint&);
    void EvMouseMove(uint, TPoint&);
    void EvLButtonUp(uint, TPoint&);
    void Paint(TDC&, bool, TRect&);
    void CmFileNew();
    void CmFileOpen();
    void CmFileSave();
    void CmFileSaveAs();
    void CmPenSize();
    void CmPenColor();
    void CmAbout();
    void SaveFile();
    void OpenFile();

    DECLARE_RESPONSE_TABLE(TDrawWindow);
};

DEFINE_RESPONSE_TABLE1(TDrawWindow, TWindow)
  EV_WM_LBUTTONDOWN,
  EV_WM_RBUTTONDOWN,
  EV_WM_MOUSEMOVE,
  EV_WM_LBUTTONUP,
  EV_COMMAND(CM_FILENEW, CmFileNew),
  EV_COMMAND(CM_FILEOPEN, CmFileOpen),
  EV_COMMAND(CM_FILESAVE, CmFileSave),
  EV_COMMAND(CM_FILESAVEAS, CmFileSaveAs),
  EV_COMMAND(CM_PENSIZE, CmPenSize),
  EV_COMMAND(CM_PENCOLOR, CmPenColor),
  EV_COMMAND(CM_ABOUT, CmAbout),
END_RESPONSE_TABLE;

TDrawWindow::TDrawWindow(TWindow *parent)
{
  Init(parent, 0, 0);
  DragDC = 0;
  Lines = new TLines(5, 0, 5);
  Line = new TLine(TColor::Black, 1);
  IsNewFile = true;
  IsDirty = false;
  FileData = new TOpenSaveDialog::TData(OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
                                        "�_��̧�� (*.PTS)|*.pts|", 0, "",
                                        "PTS");
}

bool TDrawWindow::CanClose()
{
  if (IsDirty)
    switch(MessageBox("�ۑ����܂���?", "���e���ύX����Ă��܂�",
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

void TDrawWindow::EvLButtonDown(uint, TPoint& point)
{
  if (!DragDC) {
    SetCapture();
    DragDC = new TClientDC(*this);
    Pen = new TPen(Line->QueryColor(), Line->QueryPenSize());
    DragDC->SelectObject(*Pen);
    DragDC->MoveTo(point);
    Line->Add(point);
    IsDirty = true;
  }
}

void TDrawWindow::EvRButtonDown(uint, TPoint&)
{
  GetPenSize();
}

void TDrawWindow::EvMouseMove(uint, TPoint& point)
{
  if (DragDC) {
    DragDC->LineTo(point);
    Line->Add(point);
  }
}

void TDrawWindow::EvLButtonUp(uint, TPoint&)
{
  if (DragDC) {
    ReleaseCapture();
    Lines->Add(*Line);
    Line->Flush();
    delete DragDC;
    delete Pen;
    DragDC = 0;
  }
}

void TDrawWindow::CmPenSize()
{
  GetPenSize();
}

void TDrawWindow::CmPenColor()
{
  TChooseColorDialog::TData colors;
  static TColor custColors[16] =
  {
    0x010101L, 0x101010L, 0x202020L, 0x303030L,
    0x404040L, 0x505050L, 0x606060L, 0x707070L,
    0x808080L, 0x909090L, 0xA0A0A0L, 0xB0B0B0L,
    0xC0C0C0L, 0xD0D0D0L, 0xE0E0E0L, 0xF0F0F0L
  };

  colors.Flags = CC_RGBINIT;
  colors.Color = TColor(Line->QueryColor());
  colors.CustColors = custColors;
  if (TChooseColorDialog(this, colors).Execute() == IDOK)
    Line->SetPen(colors.Color);
}

void TDrawWindow::GetPenSize()
{
  char inputText[6];
  int penSize = Line->QueryPenSize();

  wsprintf(inputText, "%d", penSize);
  if ((TInputDialog(this, "���̑���",
                        "�V�������̑��������:",
                        inputText,
                        sizeof(inputText))).Execute() == IDOK) {
    penSize = atoi(inputText);

    if (penSize < 1)
      penSize = 1;
  }
  Line->SetPen(penSize);
}

void TDrawWindow::Paint(TDC& dc, bool, TRect&)
{
  // ���I�u�W�F�N�g�̔z��ɂ��āC��������
  TLinesIterator i(*Lines);

  while (i)
    i++.Draw(dc);
}

void TDrawWindow::CmFileNew()
{
  if (CanClose()) {
    Line->Flush();
    Lines->Flush();
    Invalidate();
    IsDirty = false;
    IsNewFile = true;
  }
}

void TDrawWindow::CmFileOpen()
{
  if (CanClose())
    if ((TFileOpenDialog(this, *FileData)).Execute() == IDOK)
      OpenFile();
}

void TDrawWindow::CmFileSave()
{
  if (IsNewFile)
    CmFileSaveAs();
  else
    SaveFile();
}

void TDrawWindow::CmFileSaveAs()
{
  if (IsNewFile)
    strcpy(FileData->FileName, "");

  if ((TFileSaveDialog(this, *FileData)).Execute() == IDOK)
    SaveFile();
}

void TDrawWindow::CmAbout()
{
  TDialog(this, IDD_ABOUT).Execute();
}

void TDrawWindow::SaveFile()
{
  ofstream os(FileData->FileName);

  if (!os)
    MessageBox("̧�ق��J���܂���", "̧�ٴװ", MB_OK | MB_ICONEXCLAMATION);
  else {
    // �}�̒��̐��̐��������o��
    os << Lines->GetItemsInContainer();

    // ���\�[�X�̕������p���Đ�����t��������
    os << ' ' << string(*GetApplication(), IDS_FILEINFO) << '\n';

    // ���̔z��p�̔����q�𓾂�
    TLinesIterator i(*Lines);

    // �����q���L���ł����(���Ȃ킿�����܂�����Ȃ�)
    while (i)
      // �����q���猻�݂̐����R�s�[���C�z����C���N�������g����
      os << i++;

    // �V�����t�@�C�����Z�b�g���C�\�����X�V���ꂽ�Ƃ����t���O�� false �Ƃ���
    IsNewFile = IsDirty = false;
  }
}

void TDrawWindow::OpenFile()
{
  ifstream is(FileData->FileName);

  if (!is)
    MessageBox("̧�ق��J���܂���", "̧�ٴװ", MB_OK | MB_ICONEXCLAMATION);
  else {
    unsigned numLines;
    char fileinfo[100];

    Lines->Flush();
    Line->Flush();

    is >> numLines;
    is.getline(fileinfo, sizeof(fileinfo));
    Parent->SetCaption(fileinfo);

    for (int i = 0; i < numLines; i++) {
      TLine line;
      is >> line;
      Lines->Add(line);
    }
  }
  IsNewFile = IsDirty = false;
  Invalidate();
}

class TDrawApp : public TApplication {
  public:
    TDrawApp() : TApplication() {}

    void InitMainWindow();
};

void TDrawApp::InitMainWindow()
{
  // �������t���[���E�B���h�E���R���X�g���N�g����
  TDecoratedFrame* frame = new TDecoratedFrame(0, "��۰�ݸ��߯��", new TDrawWindow, true);

  // �X�e�[�^�X�o�[���R���X�g���N�g����
  TStatusBar* sb = new TStatusBar(frame, TGadget::Recessed);

  // �R���g���[���o�[���R���X�g���N�g����
  TControlBar *cb = new TControlBar(frame);
  cb->Insert(*new TButtonGadget(CM_FILENEW, CM_FILENEW, TButtonGadget::Command));
  cb->Insert(*new TButtonGadget(CM_FILEOPEN, CM_FILEOPEN, TButtonGadget::Command));
  cb->Insert(*new TButtonGadget(CM_FILESAVE, CM_FILESAVE, TButtonGadget::Command));
  cb->Insert(*new TButtonGadget(CM_FILESAVEAS, CM_FILESAVEAS, TButtonGadget::Command));
  cb->Insert(*new TSeparatorGadget);
  cb->Insert(*new TButtonGadget(CM_PENSIZE, CM_PENSIZE, TButtonGadget::Command));
  cb->Insert(*new TButtonGadget(CM_PENCOLOR, CM_PENCOLOR, TButtonGadget::Command));
  cb->Insert(*new TSeparatorGadget);
  cb->Insert(*new TButtonGadget(CM_ABOUT, CM_ABOUT, TButtonGadget::Command));

  // �t���[���̒��ɃX�e�[�^�X�o�[�ƃR���g���[���o�[��}������
  frame->Insert(*sb, TDecoratedFrame::Bottom);
  frame->Insert(*cb, TDecoratedFrame::Top);

  // ���C���E�B���h�E�ƃE�B���h�E�̃��j���[��ݒ肷��
  SetMainWindow(frame);
  GetMainWindow()->AssignMenu("COMMANDS");
}

int OwlMain(int /*argc*/, char* /*argv*/ [])
{
  return TDrawApp().Run();
}

ostream& operator <<(ostream& os, const TLine& line)
{
  // ���̒��̓_�̐��������o��
  os << line.GetItemsInContainer();

  // �y���̑����𓾂āC�����o��
  os << ' ' << line.Color << ' ' << line.PenSize;

  // �_�̔z���\�������q�𓾂�
  TPointsIterator j(line);

  // �����q���L���ł����(���Ȃ킿�C�_�̍��W���܂�����Ȃ�)
  while(j)
    // �����q�̎������_�̍��W�������o���C�z����C���N�������g����
    os << j++;
  os << '\n';

  // �X�g���[���I�u�W�F�N�g��Ԃ�
  return os;
}

istream& operator >>(istream& is, TLine& line)
{
  unsigned numPoints;
  is >> numPoints;

  COLORREF color;
  int penSize;
  is >> color >> penSize;
  line.SetPen(TColor(color), penSize);

  while (numPoints--) {
    TPoint point;
    is >> point;
    line.Add(point);
  }

  // �X�g���[���I�u�W�F�N�g��Ԃ�
  return is;
}
