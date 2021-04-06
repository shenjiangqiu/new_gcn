/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland dramninjas [at] gmail [dot] com
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
#include <atomic>

#include "SimulatorObject.h"
#include "Transaction.h"
#include "SystemConfiguration.h"
#include "MemorySystem.h"
#include "IniReader.h"
#include "Stats.h"
#include "AddressMapping.h"
namespace DRAMSim { 
class MultiChannelMemorySystem : public SimulatorObject 
{
  public: 
    MultiChannelMemorySystem(const string &dev, const string &sys, const string &pwd, unsigned 
        megsOfMemory);
    virtual ~MultiChannelMemorySystem();
    bool addTransaction(bool isWrite, uint64_t addr);
    bool willAcceptTransaction(uint64_t addr);

    void update();
    void printStats(bool finalStats=false);
    void printStatsToFile(bool finalStats, std::string fileName);
    void RegisterCallbacks(TransactionCompleteCB *readDone,
                           TransactionCompleteCB *writeDone,
                           void (*reportPower)(double bgpower,
                                               double burstpower,
                                               double refreshpower,
                                               double actprepower));

    // SST Statistics
    bool getStats( double *stat, DSIM_STAT metric );
    bool getStats( uint64_t *stat, DSIM_STAT metric );
    [[nodiscard]] unsigned get_channel_num() const{
      return channels.size();
    }
    static unsigned get_channel_id(uint64_t addr) {
      unsigned  ch,temp;
      addressMapping(addr,ch,temp,temp,temp,temp);
      return ch;
    }
  private:
    unsigned findChannelNumber(uint64_t addr);

  private:
    vector<MemorySystem*> channels; 
    unsigned stackID;
    string deviceIniFilename;
    string systemIniFilename;
    string pwd;
    unsigned megsOfMemory; 

  private:
    static std::atomic<int> stackCount;
}; //class MultiChannelMemorySystem
} //namespace DRAMSim
