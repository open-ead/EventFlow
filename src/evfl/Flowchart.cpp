#include <algorithm>
#include <evfl/Flowchart.h>
#include <evfl/Param.h>
#include <evfl/ResFlowchart.h>
#include <iterator>
#include <ore/Array.h>
#include <ore/BitUtils.h>
#include <ore/IterRange.h>
#include <ore/ResDic.h>
#include <ore/StringView.h>

namespace evfl {

int FlowchartContext::s_GlobalCounter{};

FlowchartContext::FlowchartContext() {
    m_handlers.SetOffset(ActionDoneHandler::GetListNodeOffset());
}

void FlowchartContext::Start(MetaDataPack* pack) {
    auto& obj = m_objs[m_obj_idx];
    const auto& entry_point = obj.GetFlowchart()->entry_points.Get()[m_active_entry_point_idx];

    const auto main_event_idx = entry_point.main_event_idx;
    if (main_event_idx == 0xffff)
        return;

    m_metadata_pack = pack;

    const int node_idx = AllocNode();
    auto& node = m_nodes[node_idx];
    node.m_obj = &obj;
    node.m_event_idx = main_event_idx;
    node.m_next_node_idx = -1;
    node.m_idx = node_idx;
    node.m_state = FlowchartContextNode::State::kNotInvoked;
    AllocVariablePack(node, entry_point);

    ProcessContext();
}

int FlowchartContext::AllocNode() {
    int idx = m_next_node_idx;

    if (idx == 0xffff) {
        idx = m_nodes.size();
        m_nodes.Resize(2 * m_nodes.size());
        for (int i = idx, n = m_nodes.size(); i < n - 1; ++i) {
            m_nodes[i].m_next_node_idx = i + 1;
            m_nodes[i].m_state = FlowchartContextNode::State::kFree;
        }
    }

    auto& node = m_nodes[idx];
    m_next_node_idx = node.m_next_node_idx;
    node.m_event_idx = -1;
    node.m_next_node_idx = -1;
    node.m_idx = -1;
    node.m_owns_variable_pack = false;
    node.m_obj = nullptr;
    node.m_variable_pack = nullptr;
    node.m_node_counter = ++s_GlobalCounter;
    node.m_state = FlowchartContextNode::State::kInvalid;
    ++m_num_allocated_nodes;
    return idx;
}

void FlowchartContext::AllocVariablePack(FlowchartContextNode& node,
                                         const ResEntryPoint& entry_point) {
    FreeVariablePack(node);

    if (entry_point.num_variable_defs != 0) {
        auto* pack = m_allocator.New<VariablePack>();
        pack->Init(m_allocator.GetArg(), &entry_point);
        node.m_variable_pack = pack;
        node.m_owns_variable_pack = true;
    }
}

void FlowchartContext::ProcessContext() {
    if (m_is_processing) {
        _91 = 1;
    } else {
        m_is_processing = true;
        do {
            _91 = 0;
            for (int i = 0, n = m_nodes.size(); i < n; ++i) {
                if (!m_nodes[i].IsInvalidOrFree())
                    _91 = (ProcessContextNode(i) | (_91 != 0)) & 1;
            }
        } while (_91);
        m_is_processing = false;
    }
}

FlowchartObj* FlowchartContext::FindFlowchartObj(ore::StringView name) {
    auto it = std::find_if(m_objs.begin(), m_objs.end(), [&](const FlowchartObj& obj) {
        return name == *obj.GetFlowchart()->name.Get();
    });
    return it == m_objs.end() ? nullptr : it;
}

const FlowchartObj* FlowchartContext::FindFlowchartObj(ore::StringView name) const {
    auto it = std::find_if(m_objs.begin(), m_objs.end(), [&](const FlowchartObj& obj) {
        return name == *obj.GetFlowchart()->name.Get();
    });
    return it == m_objs.end() ? nullptr : it;
}

void FlowchartContext::UnbindAll() {
    Clear();
    for (auto it = m_objs.begin(); it != m_objs.end(); ++it)
        it->GetActBinder().UnbindAll();
}

void FlowchartContext::Clear() {
    for (int i = 0, n = m_nodes.size(); i < n; ++i) {
        FreeVariablePack(m_nodes[i]);
        m_nodes[i].Reset();
        if (i < n - 1) {
            m_nodes[i].m_next_node_idx = i + 1;
            m_nodes[i].m_state = FlowchartContextNode::State::kFree;
        }
    }

    m_next_node_idx = 0;
    m_num_allocated_nodes = 0;
    m_metadata_pack = nullptr;

    while (!m_handlers.Empty()) {
        auto* handler = m_handlers.Front();
        handler->m_list_node.Erase();
        handler->m_context = nullptr;
        handler->m_obj = nullptr;
        handler->m_node_idx = -1;
        handler->m_node_counter = -1;
        handler->m_handled = false;
        handler->m_is_flowchart = true;
    }
}

void FlowchartContext::FreeVariablePack(FlowchartContextNode& node) {
    if (node.m_variable_pack && node.m_owns_variable_pack)
        m_allocator.DeleteAndNull(node.m_variable_pack);

    node.m_variable_pack = nullptr;
    node.m_owns_variable_pack = false;
}

void FlowchartContext::CopyVariablePack(FlowchartContextNode& src, FlowchartContextNode& dst) {
    FreeVariablePack(dst);
    dst.m_variable_pack = src.m_variable_pack;
    dst.m_owns_variable_pack = false;
}

ActorBinding* FlowchartContext::TrackBackArgumentActor(int node_idx, const ore::StringView& name) {
    if (node_idx == -1)
        return nullptr;

    const ore::ResMetaData* metadata;
    const MetaDataPack* metadata_pack;
    const VariablePack* variable_pack;
    FlowchartObj* obj;
    const ParamAccessor accessor{this, GetNode(node_idx).GetNextNodeIdx()};
    const auto real_name =
        accessor.TrackBackArgument(&metadata, &metadata_pack, &variable_pack, &obj, name);

    if (real_name.empty() || !metadata || !obj)
        return nullptr;

    const auto* param = metadata->Get(real_name, ore::ResMetaData::DataType::kActorIdentifier);
    if (!param)
        return nullptr;

    const ore::StringView actor_name = *param->value.actor.name.Get();
    const ore::StringView actor_sub_name = *param->value.actor.sub_name.Get();

    auto actor = obj->GetFlowchart()->actors.Get();
    for (int i = 0; i < int(obj->GetFlowchart()->num_actors); ++i, ++actor) {
        if (*actor->name.Get() == actor_name && *actor->secondary_name.Get() == actor_sub_name) {
            if (!actor->argument_name.Get()->empty())
                return nullptr;
            return &obj->GetActBinder().GetBindings()[i];
        }
    }

    return nullptr;
}

bool FlowchartContext::IsUsing(const ResFlowchart* flowchart) const {
    auto* obj = FindFlowchartObj(*flowchart->name.Get());
    return obj && obj->GetActBinder().IsUsed();
}

// NON_MATCHING: if (((state | 4) & 7) != 4) -- extremely weird check
bool FlowchartContext::IsPlaying(const ResFlowchart* flowchart) const {
    int state = 2;
    for (int i = 0, n = m_nodes.size(); i < n; ++i) {
        if (m_nodes[i].IsInvalidOrFree()) {
            state = 4;
        } else {
            const auto flowchart_name = ore::StringView(*flowchart->name.Get());
            const auto node_flowchart_name =
                ore::StringView(*m_nodes[i].m_obj->GetFlowchart()->name.Get());
            if (node_flowchart_name == flowchart_name) {
                state = 1;
            } else {
                state = 0;
            }
        }

        if (((state | 4) & 7) != 4)
            break;
    }
    return state != 2;
}

const ore::Array<ActorBinding>*
FlowchartContext::GetUsedResActors(ore::StringView flowchart_name) const {
    auto* obj = FindFlowchartObj(flowchart_name);
    if (!obj)
        return nullptr;
    return obj->GetActBinder().GetUsedResActors();
}

/// Recursively checks subflow calls in the specified entry point for missing flowcharts
/// or entry points.
/// @param result  Optional.
/// @param visited  Visited entry points (one BitArray per flowchart)
/// @param flowchart_idx   Index of the flowchart to which the entry point belongs.
/// @param entry_point_idx  Index of the entry point to be checked.
/// @returns true on success, false on failure
bool CheckSubFlowCalls(FlowchartContext::Builder::BuildResult* result,
                       const ore::IterRange<const ResFlowchart* const*>& flowcharts,
                       ore::Array<ore::BitArray>& visited, int flowchart_idx, int entry_point_idx) {
    if (visited[flowchart_idx].Test(entry_point_idx))
        return true;
    visited[flowchart_idx].Set(entry_point_idx);

    const auto* flowchart = *std::next(flowcharts.begin(), flowchart_idx);
    const auto entry_point_name =
        flowchart->entry_point_names.Get()->GetEntries()[1 + entry_point_idx].GetKey();
    const auto* entry_point = flowchart->GetEntryPoint(entry_point_name);

    for (u16 i = 0; i != entry_point->num_sub_flow_event_indices; ++i) {
        const auto& event = flowchart->events.Get()[entry_point->sub_flow_event_indices.Get()[i]];
        ore::StringView sub_flow_flowchart = *event.sub_flow_flowchart.Get();
        const ore::StringView sub_flow_entry_point = *event.sub_flow_entry_point.Get();

        auto sub_flowchart_idx = flowchart_idx;
        auto* sub_flowchart_res = flowchart;

        if (!sub_flow_flowchart.empty()) {
            const auto it =
                std::find_if(flowcharts.begin(), flowcharts.end(), [=](const ResFlowchart* f) {
                    return sub_flow_flowchart == *f->name.Get();
                });

            if (it == flowcharts.end()) {
                if (result) {
                    result->result =
                        FlowchartContext::Builder::BuildResultType::kResFlowchartNotFound;
                    result->missing_flowchart_name = sub_flow_flowchart;
                    result->missing_entry_point_name = {};
                }
                return false;
            }

            sub_flowchart_idx = std::distance(flowcharts.begin(), it);
            sub_flowchart_res = *it;

        } else {
            sub_flow_flowchart = *flowchart->name.Get();
        }

        const int sub_entry_point_idx =
            sub_flowchart_res->entry_point_names.Get()->FindIndex(sub_flow_entry_point);

        if (sub_entry_point_idx == -1) {
            if (result) {
                result->result = FlowchartContext::Builder::BuildResultType::kEntryPointNotFound;
                result->missing_flowchart_name = sub_flow_flowchart;
                result->missing_entry_point_name = sub_flow_entry_point;
            }
            return false;
        }

        if (!CheckSubFlowCalls(result, flowcharts, visited, sub_flowchart_idx, sub_entry_point_idx))
            return false;
    }

    if (result) {
        result->result = FlowchartContext::Builder::BuildResultType::kSuccess;
        result->missing_flowchart_name = {};
        result->missing_entry_point_name = {};
    }
    return true;
}

bool FlowchartContext::Builder::SetEntryPoint(const ore::StringView& flowchart_name,
                                              const ore::StringView& entry_point_name) {
    return SetEntryPoint(nullptr, flowchart_name, entry_point_name);
}

bool FlowchartContext::Builder::SetEntryPoint(BuildResult* result,
                                              const ore::StringView& flowchart_name,
                                              const ore::StringView& entry_point_name) {
    const auto* flowchart_it =
        std::find_if(m_flowcharts.begin(), m_flowcharts.end(), [=](const ResFlowchart* flowchart) {
            return flowchart_name == *flowchart->name.Get();
        });

    if (flowchart_it == m_flowcharts.end()) {
        if (result) {
            result->result = BuildResultType::kResFlowchartNotFound;
            result->missing_flowchart_name = flowchart_name;
            result->missing_entry_point_name = {};
        }
        return false;
    }

    const auto entry_point_idx =
        (*flowchart_it)->entry_point_names.Get()->FindIndex(entry_point_name);

    if (entry_point_idx == -1) {
        if (result) {
            result->result = BuildResultType::kEntryPointNotFound;
            result->missing_flowchart_name = flowchart_name;
            result->missing_entry_point_name = entry_point_name;
        }
        return false;
    }

    m_flowchart_idx = std::distance(m_flowcharts.begin(), flowchart_it);
    m_entry_point_idx = entry_point_idx;
    if (result) {
        result->result = BuildResultType::kSuccess;
        result->missing_flowchart_name = {};
        result->missing_entry_point_name = {};
    }
    return true;
}

bool FlowchartContext::Builder::BuildImpl(BuildResult* result, FlowchartRange flowcharts,
                                          FlowchartContext* context, AllocateArg allocate_arg,
                                          ore::Buffer flowchart_obj_buffer) {
    context->m_nodes.Reset();
    EvflAllocator allocator{allocate_arg};
    context->m_allocator = allocator;

    const int num_flowcharts = flowcharts.size();

    ore::Array<ore::BitArray> visited_entry_points;
    visited_entry_points.SetBuffer(num_flowcharts, &allocator);
    visited_entry_points.UninitializedDefaultConstructElements();

    const auto clean_up_visited_entry_points = [&] {
        if (auto* data = visited_entry_points.data()) {
            for (auto it = data; it != data + visited_entry_points.size(); ++it)
                it->FreeBufferIfNeeded(&allocator);

            visited_entry_points.DestructElements();
            allocator.Free(data);
        }
    };

    for (int i = 0; i < num_flowcharts; ++i) {
        visited_entry_points[i].AllocateBuffer(&allocator,
                                               (flowcharts.begin()[i])->num_entry_points);
    }

    if (!CheckSubFlowCalls(result, flowcharts, visited_entry_points, m_flowchart_idx,
                           m_entry_point_idx)) {
        clean_up_visited_entry_points();
        return false;
    }

    context->m_objs.ConstructElements(flowchart_obj_buffer);

    for (int i = 0; i < num_flowcharts; ++i) {
        auto* obj = &context->m_objs[i];
        FlowchartObj::Builder obj_builder{flowcharts.begin()[i], &visited_entry_points[i]};

        if (!obj_builder.Build(obj, &context->m_allocator, flowcharts)) {
            clean_up_visited_entry_points();
            context->m_objs.ClearWithoutFreeing();

            if (result) {
                const auto* flowchart = flowcharts.begin()[m_flowchart_idx];
                const ore::StringView name = *flowchart->name.Get();
                const ore::StringView ep_name = flowchart->GetEntryPointName(m_entry_point_idx);
                result->result = BuildResultType::kInvalidOperation;
                result->missing_flowchart_name = name;
                result->missing_entry_point_name = ep_name;
            }
            return false;
        }
    }

    context->m_obj_idx = m_flowchart_idx;
    context->m_active_entry_point_idx = m_entry_point_idx;
    context->m_nodes.Init(&context->m_allocator, 16);
    context->m_nodes.Resize(16);
    context->Clear();
    clean_up_visited_entry_points();

    if (result) {
        result->result = BuildResultType::kSuccess;
        result->missing_flowchart_name = {};
        result->missing_entry_point_name = {};
    }
    return true;
}

bool FlowchartContext::Builder::Build(FlowchartContext* context, AllocateArg allocate_arg) {
    return Build(nullptr, context, allocate_arg);
}

bool FlowchartContext::Builder::Build(BuildResult* result, FlowchartContext* context,
                                      AllocateArg allocate_arg) {
    context->Dispose();

    EvflAllocator allocator{allocate_arg};

    ore::DynArrayList<const ResFlowchart*> flowcharts{&allocator};
    flowcharts.Init(&allocator);
    flowcharts.DeduplicateCopy(m_flowcharts);

    FlowchartRange range{flowcharts};
    ore::Buffer obj_buffer{};
    obj_buffer.Allocate<FlowchartObj>(&allocator, flowcharts.size());

    if (!BuildImpl(result, range, context, allocate_arg, obj_buffer)) {
        obj_buffer.Free(&allocator);
        return false;
    }

    return true;
}

}  // namespace evfl
