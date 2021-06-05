#pragma once

#include <evfl/EvflAllocator.h>
#include <evfl/ResActor.h>
#include <ore/Array.h>
#include <ore/EnumUtil.h>
#include <ore/IntrusiveList.h>
#include <ore/IterRange.h>
#include <ore/StringView.h>
#include <ore/Types.h>

namespace ore {
class Allocator;
class BitArray;
}  // namespace ore

namespace evfl {

class MetaDataPack;
struct ResEntryPoint;
struct ResEvent;
struct ResFlowchart;
class VariablePack;

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
    const ActBinder& GetActBinder() const { return m_act_binder; }
    ActBinder& GetActBinder() { return m_act_binder; }

private:
    const ResFlowchart* m_flowchart{};
    ActBinder m_act_binder{};
};

class FlowchartContextNode {
public:
    ORE_ENUM(State, kInvalid, kFree, kNotInvoked, kInvoked, kDone, kWaiting)

    FlowchartContextNode() { Reset(); }

    bool IsInvalidOrFree() const { return m_state == State::kInvalid || m_state == State::kFree; }

    void Reset() {
        m_node_counter = -1;
        m_obj = nullptr;
        m_event_idx = -1;
        m_next_node_idx = -1;
        m_idx = -1;
        m_state = State::kInvalid;
        m_variable_pack = nullptr;
        m_owns_variable_pack = false;
    }

private:
    friend class FlowchartContext;

    FlowchartObj* m_obj;
    VariablePack* m_variable_pack;
    int m_node_counter;
    u16 m_event_idx;
    u16 m_next_node_idx;
    u16 m_idx;
    ore::SizedEnum<State::Type, u8> m_state;
    bool m_owns_variable_pack;
};

class FlowchartContext {
public:
    // FIXME
    class Builder {
    public:
        ORE_ENUM(BuildResultType, kSuccess, kInvalidOperation, kResFlowchartNotFound, kEntryPointNotFound)
    };

    FlowchartContext();
    ~FlowchartContext() { Dispose(); }

    FlowchartContext(const FlowchartContext&) = delete;
    auto operator=(const FlowchartContext&) = delete;

    void Start(MetaDataPack* pack);
    int AllocNode();
    void AllocVariablePack(FlowchartContextNode& node, const ResEntryPoint& entry_point);
    void ProcessContext();
    FlowchartObj* FindFlowchartObj(ore::StringView name);
    const FlowchartObj* FindFlowchartObj(ore::StringView name) const;
    void UnbindAll();
    void Clear();
    void FreeVariablePack(FlowchartContextNode& node);
    void CopyVariablePack(FlowchartContextNode& src, FlowchartContextNode& dst);
    bool ProcessContextNode(int idx);
    ActorBinding* TraceBackArgumentActor(int node_idx, const ore::StringView& name);
    bool IsUsing(const ResFlowchart* flowchart) const;
    bool IsPlaying(const ResFlowchart* flowchart) const;
    const ore::Array<ActorBinding>* GetUsedResActors(ore::StringView flowchart_name) const;

private:
    void Dispose() {
        m_obj_idx = -1;
        m_active_entry_point_idx = -1;
        m_metadata_pack = nullptr;
        m_objs.clear(&m_allocator);
        for (auto& node : m_nodes)
            FreeVariablePack(node);
        m_nodes.Reset();
    }

    static int s_GlobalCounter;

    EvflAllocator m_allocator;
    ore::Array<FlowchartObj> m_objs;
    ore::DynArrayList<FlowchartContextNode> m_nodes;
    ore::IntrusiveList<ActionDoneHandler> m_handlers;
#ifdef EVFL_VER_LABO
    void (*m_callback)(FlowchartContext* context, const ResFlowchart* flowchart,
                       const ResEvent* event, bool) = nullptr;
#endif
    MetaDataPack* m_metadata_pack = nullptr;
    void* _78 = nullptr;
    int m_next_node_idx = 0;
    int m_num_allocated_nodes = 0;
    int m_obj_idx = -1;
    int m_active_entry_point_idx = -1;
    bool m_is_processing = false;
    u8 _91 = 0;
};

}  // namespace evfl
