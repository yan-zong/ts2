//***************************************************************************
// * File:        This file is part of TS2.
// * Created on:  07 Dov 2016
// * Author:      Yan Zong, Xuweu Dai
// *
// * Copyright:   (C) 2016 Northumbria University, UK.
// *
// *              TS2 is free software; you can redistribute it and/or modify it
// *              under the terms of the GNU General Public License as published
// *              by the Free Software Foundation; either version 3 of the
// *              License, or (at your option) any later version.
// *
// *              TS2 is distributed in the hope that it will be useful, but
// *              WITHOUT ANY WARRANTY; without even the implied warranty of
// *              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// *              GNU General Public License for more details.
// *
// * Funding:     This work was financed by the Northumbria University Faculty
//                Funded and RDF funded studentship, UK
// ****************************************************************************

#ifndef __MYTDMAMAC_IIRFILTER_H_
#define __MYTDMAMAC_IIRFILTER_H_

#include <omnetpp.h>
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "IIR_types.h"

class IIRFilter : public cSimpleModule
{
    public:
        double mainIIR(double x);   // from IIR.h

    protected:
    virtual void initialize();
    // virtual void handleMessage(cMessage *msg);
    // int iirFilter(int argc, const char * const argv[]);
    // double argInit_real_T(void);
    // double main_IIR(void);
    double Nondirect_stepImpl(dspcodegen_BiquadFilter *obj, double varargin_1); // from Nondirect.h
    void IIR_initialize(void);  // from IIR_initialize.h
    void rt_InitInfAndNaN(size_t realSize); // from rt_nonfinite.h
    real_T rtGetNaN(void);  // from rtGetNaN.h
    real32_T rtGetNaNF(void);   // from rtGetNaN.h

    // from rtGetInf.h
    // *******************************************************************
    real_T rtGetInf(void);
    real32_T rtGetInfF(void);
    real_T rtGetMinusInf(void);
    real32_T rtGetMinusInfF(void);
    // *******************************************************************

    // from rt_nonfinite.h
    // *******************************************************************
    boolean_T rtIsInf(real_T value);
    boolean_T rtIsInfF(real32_T value);
    boolean_T rtIsNaN(real_T value);
    boolean_T rtIsNaNF(real32_T value);
    // *******************************************************************

    dspcodegen_BiquadFilter Hd; // from IIR_data.h

    // from rt_nonfinite.h
    // *******************************************************************
    real_T rtInf;
    real_T rtMinusInf;
    real_T rtNaN;
    real32_T rtInfF;
    real32_T rtMinusInfF;
    real32_T rtNaNF;

    typedef struct {
      struct {
        uint32_T wordH;
        uint32_T wordL;
      } words;
    } BigEndianIEEEDouble;

    typedef struct {
      struct {
        uint32_T wordL;
        uint32_T wordH;
      } words;
    } LittleEndianIEEEDouble;

    typedef struct {
      union {
        real32_T wordLreal;
        uint32_T wordLuint;
      } wordL;
    } IEEESingle;
    // *******************************************************************












};

#endif
