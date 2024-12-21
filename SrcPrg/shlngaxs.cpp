/********************************************************
   Shoaling Wave  ( for Drawing Axis)  shlingaxs.cpp
             1998/06/10 by tonto

   this Program is based on

*    Advanced User Graphic Function (Turbo-C ver.2.0)
*        spaceaxis (spaceaxi.c, ugraph1.h)
*              94/08/29 by tonto
*              94/09/04 extended for log scale
********************************************************/

#include <owl/chooseco.h>
#include <owl/color.h>
#include <classlib/arrays.h>
#include <stdio.h>
#include "shoaling.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <user\uio.h>
#include <user\umath.h>



/*  Grobal Variables in this file  */
static  float  aroundspace, defaulttextsize, defaultmarkersize, branchscale ;
static  int    nmaxaxisbranch ;
static  float  GFileScale = 1.0 ;
static  float  GraphicsViewPointX0, GraphicsViewPointY0, GraphicsViewPointX1, GraphicsViewPointY1 ;
static  float  World2ViewXA, World2ViewXB, World2ViewYA, World2ViewYB ;


static  void  readiniparameters ( void )
  {
    aroundspace       = 0.1 ;
    nmaxaxisbranch    = 5 ;
    defaulttextsize   = 0.05 ;
    defaultmarkersize = 0.02 ;
    branchscale       = 0.05 ;
  }


void  gview ( float x0, float y0, float x1, float y1 )
  {
    GraphicsViewPointX0 = x0 * GFileScale ;
    GraphicsViewPointY0 = y0 * GFileScale ;
    GraphicsViewPointX1 = x1 * GFileScale ;
    GraphicsViewPointY1 = y1 * GFileScale ;
  }


void gwindow ( float x0, float y0, float x1, float y1 )
  {
    World2ViewXA = ( GraphicsViewPointX1 - GraphicsViewPointX0 ) / ( x1 - x0 ) ;
    World2ViewXB = GraphicsViewPointX0 - World2ViewXA * x0 ;

    World2ViewYA = ( GraphicsViewPointY1 - GraphicsViewPointY0 ) / ( y1 - y0 ) ;
    World2ViewYB = GraphicsViewPointY0 - World2ViewYA * y0 ;
  }


static  float  calcpitch ( float xlen, float factor )
  {
    #define EPS  1.0e-4

    float  pitch ;

    pitch = xlen * ( 1.0 + EPS ) / nmaxaxisbranch ;
    pitch = pow10 ( floor ( log10 ( pitch / factor ) ) ) * factor ;

    return ( pitch ) ;

    #undef  EPS
  }

static  float  setbranchpara ( float xmin, float xmax,
                               float dbranch, int *branch0p, int *nbranchp )
  {
    float  branch0, branch1 ;

    branch0 = (int)(xmin/dbranch) * dbranch ;
    if ( branch0 < xmin )
      branch0 += dbranch ;
    branch1 = (int)(xmax/dbranch) * dbranch ;
    if ( branch0 > xmax )
      branch1 -= dbranch ;

    *nbranchp = (int)( (branch1-branch0)/dbranch + 0.5 ) + 1 ;
    *branch0p = round ( branch0 / dbranch ) ;

    return ( dbranch ) ;
  }


static  void  setnorpitch ( float xmin, float xmax, int *branch0p, float *dbranchp, int *nbranchp,
                            int *number0p, float *dnumberp, int *nnumberp )
  {
    int    ipitch ;
    float  pitch[3] ;
    float  numpitch[3] = { 5.0, 5.0, 2.0 } ;

    pitch[0] = calcpitch ( xmax - xmin, 1.0 ) ;
    pitch[1] = calcpitch ( xmax - xmin, 2.0 ) ;
    pitch[2] = calcpitch ( xmax - xmin, 5.0 ) ;

    ipitch = afmaxp ( pitch, 3 ) ;
    *dbranchp = setbranchpara ( xmin, xmax, pitch[ipitch],   branch0p, nbranchp ) ;
    *dnumberp = setbranchpara ( xmin, xmax, pitch[ipitch]*numpitch[ipitch], number0p, nnumberp ) ;
  }


static  void  setlogpitch ( float xmin, float xmax, int *branch0p, float *dbranchp, int *nbranchp,
                            int *number0p, float *dnumberp, int *nnumberp )
  {
    setnorpitch ( xmin, xmax, branch0p, dbranchp, nbranchp, number0p, dnumberp, nnumberp ) ;

    if ( *dbranchp < 1.0 )
      {
        *dbranchp = setbranchpara ( xmin, xmax, 1.0, branch0p, nbranchp ) ;
        *dnumberp = *dbranchp ;
        *number0p = *branch0p ;
        *nnumberp = *nbranchp ;
      }
  }


void  TShoalingGraData::spacelogaxis ( float xmin, float xmax, float ymin, float ymax, TColor &color, char *yTitle, bool xflag )
  {
    float  scalepos, xys ;
    float  dbranch, dnumber ;
    int    branch0, number0, nbranch, nnumber ;
    float  lbranchv, lbranch ;
    int    i, j ;

    readiniparameters () ;

    // ----  for Log Scale (X)  ----
    xmin = log10 ( xmin ) ;
    xmax = log10 ( xmax ) ;

//    setviewspace ( xlen, ylen, drawpos ) ;
    gwindow ( xmin, ymin, xmax, ymax ) ;

    float  xlen = fabs ( ( xmax - xmin ) * World2ViewXA / GFileScale ) ;
    float  ylen = fabs ( ( ymax - ymin ) * World2ViewYA / GFileScale ) ;

    defaulttextsize  /=  World2ViewXA / GFileScale ;

    lbranchv = branchscale * min ( xlen, ylen ) ;

    if ( xflag )
      {
        /** Draw X axis **/

//        if ( (ymax*ymin) < 0.0 )
//            scalepos = 0.0 ;
//          else
            scalepos = ymin ;

        gline ( xmin, scalepos, xmax, scalepos, TColor::Black ) ;

        lbranch = lbranchv / ( World2ViewYA / GFileScale ) ;

        // ---- for Log Axis (X)  ----
          setlogpitch ( min(xmin,xmax), max(xmin,xmax),
                        &branch0, &dbranch, &nbranch, &number0, &dnumber, &nnumber ) ;
          for ( i=0; i<nbranch; ++i )
            glinear ( ( branch0 + i ) * dbranch, scalepos, 0.0, lbranch, TColor::Black ) ;
          if ( dbranch == 1.0 )
            for ( i=-1; i<(nbranch+1); ++i )
              for ( j=1; j<10; ++j )
                {
                  xys = ( branch0 + i ) * dbranch + log10( j ) ;
                  if ( (xys-xmin)*(xys-xmax) <= 0.0 )
                    glinear ( xys, scalepos, 0.0, lbranch/2.0, TColor::Black ) ;
                }
//          gtextset ( defaulttextsize, 7, 0.0 ) ;
          for ( i=0; i<nnumber; ++i )
            {
              xys = ( number0 + i ) * dnumber ;
              gprintf ( xys, scalepos + lbranch, TA_TOP | TA_CENTER, TColor::Black, "%g", pow ( 10.0, xys ) ) ;
            }

          gprintf ( xmax, scalepos + 3*lbranch, TA_RIGHT, TColor::Black, "…[(m)" ) ;
      }


    /** Draw Y axis **/

//    if ( (xmax*xmin) < 0.0 )
//        scalepos = 0.0 ;
//      else
        scalepos = xmin ;

    gline ( scalepos, ymin, scalepos, ymax, color ) ;

    lbranch = - lbranchv / ( World2ViewXA / GFileScale ) ;

    //  ----  for Linear Scale (Y) ----
          setnorpitch ( min(ymin,ymax), max(ymin,ymax),
                        &branch0, &dbranch, &nbranch, &number0, &dnumber, &nnumber ) ;
          for ( i=0; i<nbranch; ++i )
            glinear ( scalepos, ( branch0 + i ) * dbranch, lbranch, 0.0, color ) ;
//          gtextset ( defaulttextsize, 5, 0.0 ) ;
          for ( i=0; i<nnumber; ++i )
            {
              xys = ( number0 + i ) * dnumber ;
              gprintf ( scalepos + lbranch, xys, TA_RIGHT, color, "%g", xys ) ;
            }
          gprintf ( scalepos + 3*lbranch, ymax, TA_BOTTOM , color, yTitle ) ;

  }

TPoint WPtoSP ( float x, float y )
  {
    return ( TPoint ( int(World2ViewXA * x + World2ViewXB), int(World2ViewYA * y + World2ViewYB ) ) ) ;
  }


void  TShoalingGraData::arrayline ( float *x, float *y, int n, TColor &color )
  {
    TLine *Line = new TLine ( color ) ;
    for ( int i=0; i<n; ++i )
      Line->Add ( WPtoSP ( *(x++), *(y++) ) ) ;
    GraData->Lines->Add (*Line) ;
    delete Line ;
  }


void  TShoalingGraData::gline ( float x0, float y0, float x1, float y1, TColor color )
  {
    TLine *Line = new TLine ( color ) ;
    Line->Add ( WPtoSP ( x0, y0 ) ) ;
    Line->Add ( WPtoSP ( x1, y1 ) ) ;
    GraData->Lines->Add (*Line) ;
    delete Line ;
  }


void TShoalingGraData::glinear ( float x0, float y0, float x1, float y1, TColor color )
  {
    gline ( x0, y0, x0+x1, y0+y1, color ) ;
  }


void TShoalingGraData::gprintf ( float x, float y, uint align, TColor color, char *format, float value )
  {
    char buf[32] ;
    sprintf ( buf, format, value ) ;

    TPoint pos = WPtoSP(x,y) ;
    if ( align == TA_RIGHT )
      pos += TPoint ( 0, -8 ) ;

    TGText *Text = new TGText ( pos, align, color, strdup(buf) ) ;
    GraData->Texts->Add ( *Text ) ;
    delete Text ;
  }