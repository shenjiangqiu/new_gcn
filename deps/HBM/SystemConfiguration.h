/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland 
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*  
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/



#ifndef SYSCONFIG_H
#define SYSCONFIG_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <stdint.h>
#include "PrintMacros.h"

#ifdef __APPLE__
#include <sys/types.h>
#endif

//SystemConfiguration.h
//
//Configuration values for the current system


extern bool DEBUG_TRANS_Q;
extern bool DEBUG_CMD_Q;
extern bool DEBUG_ADDR_MAP;
extern bool DEBUG_BANKSTATE;
extern bool DEBUG_BUS;
extern bool DEBUG_BANKS;
extern bool DEBUG_POWER;
extern bool VIS_FILE_OUTPUT;

extern unsigned NUM_BANKGROUPS;
extern unsigned NUM_BANKS;
extern unsigned NUM_BANKS_LOG;
extern unsigned NUM_RANKS;
extern unsigned NUM_RANKS_LOG;
extern unsigned NUM_BANKS_PER_BANKGROUP;
extern unsigned NUM_CHANS;
extern unsigned NUM_CHANS_LOG;
extern unsigned NUM_ROWS;
extern unsigned NUM_ROWS_LOG;
extern unsigned NUM_COLS;
extern unsigned NUM_COLS_LOG;
extern unsigned PREFETCH_SIZE;
extern unsigned PAGE_SIZE;
extern unsigned BYTE_OFFSET_WIDTH;
extern unsigned TRANSACTION_SIZE;

//in nanoseconds
extern unsigned REFRESH_PERIOD;
extern float tCK;

extern unsigned CL;
extern unsigned AL;
#define RL (CL+AL)
#define WL (RL-1)
extern unsigned BL;
extern unsigned tRAS;
extern unsigned tRCD;
extern unsigned tRRDS;
extern unsigned tRRDL;
extern unsigned tRC;
extern unsigned tRP;
extern unsigned tCCDS;
extern unsigned tCCDL;
extern unsigned tRTP;
extern unsigned tWTRS;
extern unsigned tWTRL;
extern unsigned tWR;
extern unsigned tRTRS;
extern unsigned tRFC;
extern unsigned tFAW;
extern unsigned tCKE;
extern unsigned tXP;

extern unsigned tCMD;

/* For power parameters (current and voltage), see externs in MemoryController.cpp */ 

//same bank
#define READ_TO_PRE_DELAY (AL+BL/2+ max(tRTP,tCCDS)-tCCDS)
#define WRITE_TO_PRE_DELAY (WL+BL/2+tWR)
#define READ_TO_WRITE_DELAY (RL+BL/2+tRTRS-WL)
#define READ_AUTOPRE_DELAY (AL+tRTP+tRP)
#define WRITE_AUTOPRE_DELAY (WL+BL/2+tWR+tRP)
#define WRITE_TO_READ_DELAY_R (WL+BL/2+tRTRS-RL) //interrank

extern unsigned JEDEC_DATA_BUS_BITS;

//Memory Controller related parameters
extern unsigned TRANS_QUEUE_DEPTH;
extern unsigned CMD_QUEUE_DEPTH;

extern unsigned EPOCH_LENGTH;

extern unsigned TOTAL_ROW_ACCESSES;

extern std::string ROW_BUFFER_POLICY;
extern std::string SCHEDULING_POLICY;
extern std::string ADDRESS_MAPPING_SCHEME;
extern std::string MODE;
extern bool BANK_GROUPS_ENABLED;

enum TraceType
{
  k6,
  mase,
  misc
};

enum AddressMappingScheme
{
  ChRaBaRoCo,
  RoBaRaCoCh
};

// used in MemoryController and CommandQueue
enum RowBufferPolicy
{
  OpenPage,
  ClosePage
};

enum SchedulingPolicy
{
  RankThenBankRoundRobin,
  BankThenRankRoundRobin
};

enum OperationMode
{
  LegacyMode,
  PseudoChannelMode
};

// set by IniReader.cpp


namespace DRAMSim
{
typedef void (*returnCallBack_t)(unsigned id, uint64_t addr, uint64_t clockcycle);

extern RowBufferPolicy rowBufferPolicy;
extern SchedulingPolicy schedulingPolicy;
extern AddressMappingScheme addressMappingScheme;
extern OperationMode operationMode;

unsigned inline log2(unsigned value)
{
  unsigned logbase2 = 0;
  unsigned orig = value;
  value>>=1;
  while (value > 0) {
    value >>= 1;
    logbase2++;
  }

  if ((unsigned)1<<logbase2 < orig)
    logbase2++;

  return logbase2;
}

inline bool isPowerOfTwo(uint64_t x)
{
  return ((1UL<<log2(x)) == x);
}
};

#endif

