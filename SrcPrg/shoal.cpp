#include <owl/chooseco.h>
#include <owl/color.h>
#include <classlib/arrays.h>
#include <stdio.h>
#include "shoaling.h"

#include <math.h>
#include <user\uio.h>
#include <user\cesp.h>
#include <user\umath.h>


WaveInfo::WaveInfo ( float H0s, float T0s, float slope )
  {
    /* Set Initial Data  */
    H0 = H0s ;
    T = T0s ;
    tanbeta = 1.0 / slope ;

    nData = 300 ;
    h = new float[nData] ;
    logh = new float[nData] ;
    H = new float[nData] ;
    Hs = new float[nData] ;
    c = new float[nData] ;
    L = new float[nData] ;

    iBreak = -1 ;
  }


WaveInfo::~WaveInfo ()
  {
    delete h ;
    delete logh ;
    delete H ;
    delete Hs ;
    delete c ;
    delete L ;
  }


void WaveInfo::SetDepth ( void )
  {
    L0 = Grav * sqr(T) / ( 2.0 * pi ) ;
    
    float loghmax = ceil ( log10 ( L0 / 2.0 ) ) ;
    float loghmin = floor ( log10 ( L0 / 200.0 ) ) ;
    
    for ( int i=0; i<nData; ++i )
      {
        logh[i] = ( loghmax * (nData-1-i) + loghmin * i ) / (nData-1) ;
        h[i] = pow ( 10.0, logh[i] ) ;
      }
  }


/****  Calculation of Wave Height as Small Amplitude Waves  ****/
void  WaveInfo::CalcHs ( void )
  {
    float kh, tanhkh, Ks ;
    
    for ( int i=0; i<nData; ++i )
      {
        kh = k ( h[i], T ) * h[i] ;
        tanhkh = tanh ( kh ) ;
        Ks = 1.0 / sqrt ( tanhkh + kh * ( 1.0 - sqr(tanhkh) ) ) ;
        Hs[i] = Ks * H0 ;
      }
  }


/****  Calculation of Wave Height with Goda method  ****/
void  WaveInfo::CalcH ( void )
  {
    float Hb ;
    for ( int i=0; i<nData; ++i )
      {
        H[i] = Hamp ( h[i] ) ;
        Hb = Hbreak ( h[i] ) ;
        if ( H[i] >= Hb )
          {
            iBreak = i ;
            break ;
          }
      }
  }


/****  Calculation of Wave Height as Finite Amplitude Waves  ****/
float  WaveInfo::Hamp ( float h )
  {
    const float  Eps = 10.0e-5 ;

    float kh, tanhkh, Ks, Ks30, Ks50, Kss, HL02 ;
    float h30s ;

    float h30 = h ;
    do {
        h30s = h30 ;
        kh = k ( h30s, T ) * h30s ;
        tanhkh = tanh ( kh ) ;
        Ks30 = 1.0 / sqrt ( tanhkh + kh * ( 1.0 - sqr(tanhkh) ) ) ;
        HL02 = 2.0*pi / 30.0 * H0/L0 * Ks30 ;
        h30 = sqrt ( HL02 ) * L0 ;
      } while ( ( fabs ( h30 - h30s ) / h30 ) > Eps ) ;

    if ( h < h30 )
        {
          HL02 = 2.0*pi / 50.0 * H0/L0 * Ks30 ;
          float h50 = sqrt ( HL02 ) * L0 ;
          if ( h50 < h )
              Ks = Ks30 * pow ( h30/h, 2.0/7.0 ) ;
            else
              {
                Ks50 = Ks30 * pow ( h30/h50, 2.0/7.0 ) ;
                float B = 2.0 * sqrt(3.0) / sqrt ( 2.0*pi * H0/L0 ) * h/L0 ;
                float C50 = Ks50 * pow ( h50/L0, 3.0/2.0 )
                           * ( sqrt ( 2.0*pi * H0/L0 * Ks50 )
                               - 2.0 * sqrt(3.0) * h50/L0 ) ;
                float C = C50 / ( sqrt ( 2.0*pi * H0/L0 ) * pow ( h/L0, 3.0/2.0 ) ) ;

                Ks = Ks30 ;
                do {
                    Kss = Ks ;
                    Ks = ( 1.0/2.0 * pow ( Kss, 3.0/2.0 ) + C )
                         / ( 3.0/2.0 * sqrt(Kss) - B ) ;
                  } while ( fabs ( Ks - Kss ) / Ks > Eps ) ;
              }
        }
      else
        {
          kh = k ( h, T ) * h ;
          tanhkh = tanh ( kh ) ;
          Ks = 1.0 / sqrt ( tanhkh + kh * ( 1.0 - sqr(tanhkh) ) ) ;
        }

    return ( Ks * H0 ) ;
  }


/****  Criteria of Wave Breaking by Goda  ****/
float  WaveInfo::Hbreak ( float hb )
  {
    const float A = 0.17 ;

    float HbL0 = A * ( 1.0
                     - exp ( -1.5 * pi * hb / L0
                             * ( 1.0 + 15.0 * pow ( tanbeta, 4.0/3.0 ) ) ) ) ;
    return ( HbL0 * L0 ) ;
  }


/****  Calculation of Wavenumber, k, with library  ****/
float  WaveInfo::k ( float Depth, float Period )
  {
    float c = smallawave ( Depth, Period ) ;
    return ( ( 2.0 * pi / Period ) / c ) ;
  }


/****  Calculation of Wave Celerity   ****/
void  WaveInfo::Calcc ( void )
  {
    int n = nData ;
    if ( iBreak >= 0 )
      n = iBreak + 1 ;

    for ( int i=0; i<n; ++i )
      c[i] = initfawave  ( h[i], H[i], T ) ;
  }


/****  Calculation of Wave Length  ****/
void  WaveInfo::CalcL ( void )
  {
    int n = nData ;
    if ( iBreak >= 0 )
      n = iBreak + 1 ;

    for ( int i=0; i<n; ++i )
      L[i] = c[i] * T ;
  }


void  WaveInfo::Hdist ( void )
  {
    SetDepth () ;
    CalcHs () ;
    CalcH () ;
    Calcc () ;
    CalcL () ;
  }


void  WaveInfo::Hrange ( float *Hmin, float *Hmax )
  {
    float H0min = H0 ;

    for ( int i=0; i<nData; ++i )
      if ( Hs[i] < H0min )
        H0min = Hs[i] ;
    
    float H0max = Hs[nData-1] ;
    if ( H[iBreak] > H0max )
      H0max = H[iBreak] ;
    
    *Hmin = H0min ;
    *Hmax = H0max ;
  }


void  WaveInfo::OutputData ( FILE *fp )
  {
    int n = nData ;
    if ( iBreak == 0 )
      n = 0 ;
    if ( iBreak > 0 )
      n = iBreak + 1 ;


    fprintf ( fp, "êÛêÖïœå`\n\n" ) ;
    fprintf ( fp, "  â´îgîgçÇ : %g m\n", H0 ) ;
    fprintf ( fp, "  é¸ä˙     : %g s\n", T ) ;
    fprintf ( fp, "  äCíÍå˘îz : 1/%g\n\n", 1.0/tanbeta ) ;

    if ( iBreak > 0 )
        {
          fprintf ( fp, "  ç”îgîgçÇ : %g m\n", H[iBreak] ) ;
          fprintf ( fp, "  ç”îgêÖê[ : %g m\n\n", h[iBreak] ) ;
        }
      else
        {
          if ( iBreak == -1 )
              fprintf ( fp, "  ä›ë§Ç‹Ç≈ç”îgÇπÇ∏\n\n" ) ;
            else
              fprintf ( fp, "  â´ë§Ç≈Ç∑Ç≈Ç…ç”îg\n\n" ) ;
        }


    fprintf ( fp, "        êÖê[(m)         îgçÇ(m)         îgçÇ(m)       îgë¨(m/s)         îgí∑(m)\n" ) ;
    fprintf ( fp, "                   (î˜è¨êUïùîg) \n" ) ;

    for ( int i=0; i<n; ++i )
      fprintf ( fp, "%15.7g %15.7g %15.7g %15.7g %15.7g\n",
                      h[i], Hs[i]/H0, H[i]/H0, c[i], L[i] ) ;
    for ( int i=n; i<nData; ++i )
      fprintf ( fp, "%15.7g %15.7g\n", h[i]/L0, Hs[i]/H0 ) ;
  }
