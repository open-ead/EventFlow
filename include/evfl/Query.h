#pragma once

#include <evfl/ParamAccessor.h>
#include <ore/EnumUtil.h>

namespace evfl {

class FlowchartContext;
struct ResEvent;
class VariablePack;

ORE_ENUM(QueryValueType, kBool, kInt, kFloat, kString, kConst)

struct QueryArg {
    void* actor_userdata;
    void* userdata;
    ResEvent* main_event;
    FlowchartContext* flowchart_ctx;
    VariablePack* variable_pack;
    ParamAccessor param_accessor;
};

}  // namespace evfl
