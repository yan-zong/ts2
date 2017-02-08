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

// Constants for simulation
#define MAX_SLAVE 100

// Constants for PTP
// Packet length
#define SYNC_BYTE 164
#define DRES_BYTE 164 
#define DREQ_BYTE 164

#define OFFSET	  0
#define DRIFT	  1
#define CONSTDELAY 17E-3

// Broadcast and Null address for PTP packet
// Since our PTP address follows the same as the L3 address ("SimpleAddress.h")
// we use the same definition as L3 address
// -1   for broadcast address
// 0    for null address
#define PTP_BROADCAST_ADDR -1
#define PTP_NULL_ADDR 0

// Types for PTP packets PtpPkt.ptpType
enum PtpPacket_types
{
    SYNC = 0,
    DREQ = 1,
    DRES = 2,
    REG = 3,
    REGREPLY = 4
};

// Values for PtpPkt.pckType
// OTHER for packet with name "REGISTER"*/
enum
{
    PTP = 0,
    OTHER = 1,
    CLOCK = 2
};

// Types of messages exchanged between the node and clock.
enum
{
    TIME_REQ = 0,
    TIME_RES = 1,
    OFFSET_ADJ = 2,
    FREQ_ADJ = 3,
    PROCESSED_OFFSET = 4
};

// Types of events managed by the manager node.
enum
{
    CICLICO = 0,
    ACICLICO = 1
};
