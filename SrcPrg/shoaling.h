/************************************************
  Header File for Shoaling Programs
      98/06/10  by tonto
************************************************/

//********  shlngapp.cpp

typedef TArray<TPoint> TPoints;
typedef TArrayIterator<TPoint> TPointsIterator;

class TLine : public TPoints {
  public:
    // �F�ƃy���̑������w��ł���R���X�g���N�^
    // �܂��C�f�t�H���g�̃R���X�g���N�^�Ƃ��Ă����p�ł���
    TLine(const TColor &color = TColor(0), int penSize = 1)
      : TPoints(10, 0, 10), PenSize(penSize), Color(color) {}

    // TLine �́C���ꎩ�g��`�悷��B���ׂĂ��܂��������Ȃ�Ctrue ��Ԃ�
    virtual bool Draw(TDC &) const;

    // == ���Z�q�́C�g�p����Ă��Ȃ��Ă��R���e�i�N���X�p�ɒ�`���Ȃ���΂Ȃ�Ȃ�
    bool operator ==(const TLine& other) const { return &other == this; }

  protected:
    int PenSize;
    TColor Color;
};


typedef TArray<TLine> TLines;
typedef TArrayIterator<TLine> TLinesIterator;


//**************  Array of Strings for Graphics

class TGText
  {
    public:
      TGText ( const TPoint pos = TPoint(0,0), const uint align = 0,
               const TColor color = TColor(0), char *text = NULL ) ;
      ~TGText () ;

      virtual bool Draw(TDC &) const;

      // == ���Z�q�́C�g�p����Ă��Ȃ��Ă��R���e�i�N���X�p�ɒ�`���Ȃ���΂Ȃ�Ȃ�
      bool operator ==(const TGText& other) const { return &other == this; }

    protected:
      TPoint  Pos ;
      uint    aAlign ;
      TColor  Color ;
      char*   tText ;
  } ;

typedef TArray<TGText> TGTexts;
typedef TArrayIterator<TGText> TGTextsIterator;


class TGraData {
  public:
    TGraData () { Lines = new TLines (5,0,5) ; Texts = new TGTexts(5,0,5) ; } ;
    ~TGraData () { delete Lines; delete Texts ; } ;

    TLines *Lines ;
    TGTexts *Texts ;
  };


//*******************  shoal.cpp

class WaveInfo
 {
    public :
      float H0, T, L0, tanbeta ;
      int nData, iBreak ;
      float *h, *logh, *H, *Hs, *c, *L ;

      WaveInfo ( float H0s = 1.0 , float T0s = 10.0 , float slope = 100.0 ) ;
      ~WaveInfo () ;
      void  Hdist ( void ) ;
      void  SetDepth ( void ) ;
      void  CalcHs ( void ) ;
      void  CalcH ( void ) ;
      void  Calcc ( void ) ;
      void  CalcL ( void ) ;
      void  Hrange ( float *Hmin, float *Hmax ) ;
      void  OutputData ( FILE *fp ) ;

    private :
      float  k ( float Depth, float Period ) ;
      float  Hamp ( float h ) ;
      float  Hbreak ( float hb ) ;

    public :
      void  OutputData ( char *filename ) ;
  } ;


//*******  Graphics Functions

TPoint WPtoSP ( float x, float y ) ;
void  gview ( float x0, float y0, float x1, float y1 ) ;
void gwindow ( float x0, float y0, float x1, float y1 ) ;


//********  shlnggrd.cpp


class  TShoalingGraData
  {
    public:
      TShoalingGraData () ;
      ~TShoalingGraData () ;

      TGraData *GraData ;
      WaveInfo *Waves ;

      TGraData *SetGraphicData ( float H0, float T, float tanbeta ) ;
      void SetSaveData ( FILE *fp, float H0, float T, float tanbeta ) ;

    private:
      void  spacelogaxis ( float xmin, float xmax, float ymin, float ymax, TColor &color, char *yTitle, bool xflag = true ) ;
      void  arrayline ( float *x, float *y, int n, TColor &color ) ;
      void  gline ( float x0, float y0, float x1, float y1, TColor color ) ;
      void  glinear ( float x0, float y0, float x1, float y1, TColor color ) ;
      void  gprintf ( float x, float y, uint align, TColor color, char *format, float value = 0.0 ) ;
      void  ExtendRange ( float *Xmin, float *Xmax ) ;
      void  DrawTextInfo ( void ) ;
  } ;



