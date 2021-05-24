#pragma once

#include <ore/EnumUtil.h>

namespace evfl {

// FIXME
class FlowchartContextNode {
public:
    ORE_ENUM(State, kInvalid, kFree, kNotInvoked, kInvoked, kDone, kWaiting)
};

// FIXME
class FlowchartContext {
public:
    // FIXME
    class Builder {
    public:
        ORE_ENUM(BuildResultType, kSuccess, kInvalidOperation, kResFlowchartNotFound, kEntryPointNotFound)
    };
};

}  // namespace evfl
