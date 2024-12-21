//----------------------------------------------------------------------------
// ObjectWindows - (C) Copyright 1991, 1994 by Borland International
//   チュートリアル アプリケーション -- step10.cpp
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
    // 色とペンの太さを指定できるコンストラクタ
    // また，デフォルトのコンストラクタとしても利用できる
    TLine(const TColor &color = TColor(0), int penSize = 1)
      : TPoints(10, 0, 10), PenSize(penSize), Color(color) {}

    // ペンの属性をたずね，変更する関数
    int QueryPenSize() { return PenSize; }
    TColor& QueryColor() { return Color; }
    void SetPen(TColor &newColor, int penSize = 0);
    void SetPen(int penSize);

    // TLine は，それ自身を描画する。すべてうまくいったなら，true を返す
    virtual bool Draw(TDC &) const;

    // == 演算子は，使用されていなくてもコンテナクラス用に定義しなければならない
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
  // penSize がデフォルト値 (0) でないなら，PenSize に新しい線の太さを設定する
  if (penSize)
    PenSize = penSize;

  Color = newColor;
}

bool TLine::Draw(TDC &dc) const
{
  // この線のペンの値を，dc に設定する
  TPen pen(Color, PenSize);
  dc.SelectObject(pen);

  // 線 i の各点の座標について，反復する
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
    TLine *Line; // 線を１本だけ一時的に格納する。これは後で，LINES に格納しなおす
    TOpenSaveDialog::TData
              *FileData;
    bool IsDirty, IsNewFile;

    void GetPenSize(); // GetPenSize は常に Line->SetPen() を呼び出す

    // TWindow のメンバー関数をオーバーライドする
    bool CanClose();

    // メッセージ応答関数
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
                                        "点のﾌｧｲﾙ (*.PTS)|*.pts|", 0, "",
                                        "PTS");
}

bool TDrawWindow::CanClose()
{
  if (IsDirty)
    switch(MessageBox("保存しますか?", "内容が変更されています",
                      MB_YESNOCANCEL | MB_ICONQUESTION)) {
      case IDCANCEL:
        // Cancel を選択するとウィンドウを閉じるのを止める -- false を返す
        return false;

      case IDYES:
        // Yes を選択すると描画内容を保存する
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
  if ((TInputDialog(this, "線の太さ",
                        "新しい線の太さを入力:",
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
  // 線オブジェクトの配列について，反復する
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
    MessageBox("ﾌｧｲﾙが開けません", "ﾌｧｲﾙｴﾗｰ", MB_OK | MB_ICONEXCLAMATION);
  else {
    // 図の中の線の数を書き出す
    os << Lines->GetItemsInContainer();

    // リソースの文字列を用いて説明を付け加える
    os << ' ' << string(*GetApplication(), IDS_FILEINFO) << '\n';

    // 線の配列用の反復子を得る
    TLinesIterator i(*Lines);

    // 反復子が有効である間(すなわち線がまだあるなら)
    while (i)
      // 反復子から現在の線をコピーし，配列をインクリメントする
      os << i++;

    // 新しいファイルをセットし，表示が更新されたというフラグを false とする
    IsNewFile = IsDirty = false;
  }
}

void TDrawWindow::OpenFile()
{
  ifstream is(FileData->FileName);

  if (!is)
    MessageBox("ﾌｧｲﾙが開けません", "ﾌｧｲﾙｴﾗｰ", MB_OK | MB_ICONEXCLAMATION);
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
  // 装飾つきフレームウィンドウをコンストラクトする
  TDecoratedFrame* frame = new TDecoratedFrame(0, "ﾄﾞﾛｰｲﾝｸﾞﾊﾟｯﾄﾞ", new TDrawWindow, true);

  // ステータスバーをコンストラクトする
  TStatusBar* sb = new TStatusBar(frame, TGadget::Recessed);

  // コントロールバーをコンストラクトする
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

  // フレームの中にステータスバーとコントロールバーを挿入する
  frame->Insert(*sb, TDecoratedFrame::Bottom);
  frame->Insert(*cb, TDecoratedFrame::Top);

  // メインウィンドウとウィンドウのメニューを設定する
  SetMainWindow(frame);
  GetMainWindow()->AssignMenu("COMMANDS");
}

int OwlMain(int /*argc*/, char* /*argv*/ [])
{
  return TDrawApp().Run();
}

ostream& operator <<(ostream& os, const TLine& line)
{
  // 線の中の点の数を書き出す
  os << line.GetItemsInContainer();

  // ペンの属性を得て，書き出す
  os << ' ' << line.Color << ' ' << line.PenSize;

  // 点の配列を表す反復子を得る
  TPointsIterator j(line);

  // 反復子が有効である間(すなわち，点の座標がまだあるなら)
  while(j)
    // 反復子の示した点の座標を書き出し，配列をインクリメントする
    os << j++;
  os << '\n';

  // ストリームオブジェクトを返す
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

  // ストリームオブジェクトを返す
  return is;
}
