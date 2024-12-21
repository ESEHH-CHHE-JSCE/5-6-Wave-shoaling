/*************************************************
     Setup Graphic Data  shlnggrd.cpp
           1998/06/10  by tonto
*************************************************/

#include <owl/chooseco.h>
#include <owl/color.h>
#include <classlib/arrays.h>
#include <stdio.h>
#include "shoaling.h"

const int XminScrn = 200 ;
const int XmaxScrn = 700 ;
const int YminScrn = 400 ;
const int YmaxScrn = 50 ;
const int DeltaXScrn = 70 ;


TShoalingGraData::TShoalingGraData ()
  {
    GraData = new TGraData () ;

    gview ( XminScrn, YminScrn, XmaxScrn, YmaxScrn ) ;
    gwindow ( XminScrn, YminScrn, XmaxScrn, YmaxScrn ) ;
  }


TShoalingGraData::~TShoalingGraData ()
  {
//    delete GraData ;
  }

void TShoalingGraData::ExtendRange ( float *Xmin, float *Xmax )
  {
    const float ExtPara = 0.1 ;

    float dx = *Xmax - *Xmin ;
    *Xmax += ExtPara * dx ;
    *Xmin -= ExtPara * dx ;
    if ( *Xmin < 0.0 )
      *Xmin = 0.0 ;
  }

void  TShoalingGraData::DrawTextInfo ( void )
  {
    #define  nlines 16

    gview ( XminScrn-DeltaXScrn, YminScrn, XmaxScrn-DeltaXScrn, YmaxScrn ) ;
    gwindow ( 0.0, -3.0, 2.0, -nlines-3.0 ) ;
    gprintf ( 0.0, 1.0, TA_LEFT | VTA_CENTER, TColor::Black, "‰«”g”g‚ %gm", Waves->H0 ) ;
    gprintf ( 0.0, 2.0, TA_LEFT | VTA_CENTER, TColor::Black, "ŽüŠú %gs", Waves->T ) ;
    gprintf ( 0.0, 3.0, TA_LEFT | VTA_CENTER, TColor::Black, "’ê–ÊŒù”z 1/%g", 1.0/Waves->tanbeta ) ;

    if ( Waves->iBreak > 0 )
        {
          gprintf ( 1.0, 1.0, TA_LEFT | VTA_CENTER, TColor::Black, "Ó”g”g‚ %gm", Waves->H[Waves->iBreak] ) ;
          gprintf ( 1.0, 2.0, TA_LEFT | VTA_CENTER, TColor::Black, "Ó”g…[ %gm", Waves->h[Waves->iBreak] ) ;
        }
      else
        {
          if ( Waves->iBreak == 0 )
              gprintf ( 1.0, 1.0, TA_LEFT | VTA_CENTER, TColor::Black, "‰«‘¤‚Å‚·‚Å‚ÉÓ”g" ) ;
            else  
              gprintf ( 1.0, 1.0, TA_LEFT | VTA_CENTER, TColor::Black, "ŠÝ‘¤‚Ü‚ÅÓ”g‚¹‚¸" ) ;
        }

    #undef nlines
  }


TGraData *TShoalingGraData::SetGraphicData ( float H0, float T, float slope )
  {
    Waves = new WaveInfo ( H0, T, slope ) ;
    Waves->Hdist () ;

    float Hmin, Hmax ;
    Waves->Hrange ( &Hmin, &Hmax ) ;
    ExtendRange ( &Hmin, &Hmax ) ;

    gview ( XminScrn, YminScrn, XmaxScrn, YmaxScrn ) ;
    spacelogaxis ( Waves->h[Waves->nData-1], Waves->h[0], Hmin, Hmax, TColor::LtRed, "”g‚(m)" ) ;
    arrayline ( Waves->logh, Waves->Hs, Waves->nData, TColor::LtRed ) ;
    gprintf ( Waves->logh[Waves->nData-1]+0.1, Waves->Hs[Waves->nData-1], TA_LEFT | TA_BOTTOM, TColor::LtRed, "”÷¬U•”g—˜_" ) ;
    int n = Waves->iBreak ;
    if ( n < 0 )
      n = Waves->nData-1 ;

    arrayline ( Waves->logh, Waves->H, n+1, TColor::LtRed ) ;

    float cmax = Waves->c[0] ;
    float cmin = 0.0 ;
    ExtendRange ( &cmin, &cmax ) ;

    gview ( XminScrn-DeltaXScrn, YminScrn, XmaxScrn-DeltaXScrn, YmaxScrn ) ;
    spacelogaxis ( Waves->h[Waves->nData-1], Waves->h[0], cmin, cmax, TColor::LtCyan, "”g‘¬(m/s)", false ) ;
    gview ( XminScrn, YminScrn, XmaxScrn, YmaxScrn ) ;
    gwindow (Waves->logh[Waves->nData-1], cmin, Waves->logh[0], cmax ) ;
    arrayline ( Waves->logh, Waves->c, n+1, TColor::LtCyan ) ;

    float Lmax = Waves->L[0] ;
    float Lmin = 0.0 ;
    ExtendRange ( &Lmin, &Lmax ) ;

    gview ( XminScrn-2*DeltaXScrn, YminScrn, XmaxScrn-2*DeltaXScrn, YmaxScrn ) ;
    spacelogaxis ( Waves->h[Waves->nData-1], Waves->h[0], Lmin, Lmax, TColor::LtCyan, "”g’·(m)", false ) ;
    gview ( XminScrn, YminScrn, XmaxScrn, YmaxScrn ) ;
    gwindow (Waves->logh[Waves->nData-1], Lmin, Waves->logh[0], Lmax ) ;
    arrayline ( Waves->logh, Waves->L, n+1, TColor::LtCyan ) ;

    DrawTextInfo () ;

    delete Waves ;

    return ( GraData ) ;
  }


void TShoalingGraData::SetSaveData ( FILE *fp, float H0, float T, float slope )
  {
    Waves = new WaveInfo ( H0, T, slope ) ;
    Waves->Hdist () ;
    Waves->OutputData ( fp ) ;
    delete Waves ;
  }