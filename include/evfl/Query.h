#pragma once

#include <evfl/ParamAccessor.h>

namespace evfl {

class FlowchartContext;
struct ResEvent;
class VariablePack;

struct QueryArg {
    void* actor_userdata;
    void* userdata;
    ResEvent* main_event;
    FlowchartContext* flowchart_ctx;
    VariablePack* variable_pack;
    ParamAccessor param_accessor;
};

}  // namespace evfl
