/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * IIR_initialize.c
 *
 * Code generation for function 'IIR_initialize'
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "IIR.h"
#include "IIR_initialize.h"
#include "IIR_data.h"

/* Function Definitions */
void iir_initialize(void)
{
  dspcodegen_BiquadFilter *obj;
  int i;
  static const signed char iv0[12] = { 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1 };

  static const double dv0[8] = { -0.514291170370754, 0.686625731873651,
    -0.398823703021892, 0.307948412789063, -0.340340188506195, 0.116150835549945,
    -0.315316364829903, 0.0340848243993438 };

  static const double dv1[5] = { 0.293083640375724, 0.227281177441793,
    0.193952661760938, 0.17969211489236, 0.0 };

  static const boolean_T bv0[5] = { true, true, true, true, false };

  double b_obj;
  rt_InitInfAndNaN(8U);

  /*  The following code was used to design the filter coefficients: */
  /*  */
  /*  Fpass = 1;    % Passband Frequency */
  /*  Fstop = 2;    % Stopband Frequency */
  /*  Apass = 3;    % Passband Ripple (dB) */
  /*  Astop = 100;  % Stopband Attenuation (dB) */
  /*  Fs    = 5;    % Sampling Frequency */
  /*  */
  /*  h = fdesign.lowpass('fp,fst,ap,ast', Fpass, Fstop, Apass, Astop, Fs); */
  /*  */
  /*  Hd = design(h, 'butter', ... */
  /*      'MatchExactly', 'stopband', ... */
  /*      'SystemObject', true); */
  obj = &Hd;
  Hd.isInitialized = 1;

  /* System object Constructor function: dsp.BiquadFilter */
  obj->cSFunObject.P0_ICRTP = 0.0;
  for (i = 0; i < 12; i++) {
    obj->cSFunObject.P1_RTP1COEFF[i] = iv0[i];
  }

  for (i = 0; i < 8; i++) {
    obj->cSFunObject.P2_RTP2COEFF[i] = dv0[i];
  }

  for (i = 0; i < 5; i++) {
    obj->cSFunObject.P3_RTP3COEFF[i] = dv1[i];
  }

  for (i = 0; i < 5; i++) {
    obj->cSFunObject.P4_RTP_COEFF3_BOOL[i] = bv0[i];
  }

  /* System object Initialization function: dsp.BiquadFilter */
  b_obj = obj->cSFunObject.P0_ICRTP;
  for (i = 0; i < 8; i++) {
    obj->cSFunObject.W0_FILT_STATES[i] = b_obj;
  }
}

/* End of code generation (IIR_initialize.c) */
