//
// Created by sjq on 1/6/21.
//

#ifndef GCN_SIM_SYSTEM_H
#define GCN_SIM_SYSTEM_H

#include <memory>

#include "Aggregator.h"
#include "SystolicArray.h"
#include "buffer.h"
#include "memory_interface.h"


class System {
public:
private:

    std::shared_ptr<Aggregator> m_aggregator;
    std::shared_ptr<SystolicArray> m_systolic_array;

};


#endif //GCN_SIM_SYSTEM_H
