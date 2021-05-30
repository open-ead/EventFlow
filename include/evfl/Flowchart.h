#pragma once

#include <evfl/ResActor.h>
#include <ore/EnumUtil.h>
#include <ore/IterRange.h>

namespace ore {
class Allocator;
class BitArray;
}  // namespace ore

namespace evfl {

struct ResFlowchart;

class FlowchartObj {
public:
    class Builder {
    public:
        Builder(const ResFlowchart* flowchart, ore::BitArray* visited_entry_points)
            : m_flowchart(flowchart), m_entry_points_mask(visited_entry_points) {}

        bool Build(FlowchartObj* obj, ore::Allocator* allocator,
                   ore::IterRange<const ResFlowchart* const*> flowcharts);

    private:
        const ResFlowchart* m_flowchart{};
        ore::BitArray* m_entry_points_mask{};
        ActBinder::Builder m_act_binder_builder{};
    };

    const ResFlowchart* GetFlowchart() const { return m_flowchart; }
    ActBinder& GetActBinder() { return m_act_binder; }

private:
    const ResFlowchart* m_flowchart{};
    ActBinder m_act_binder{};
};

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
