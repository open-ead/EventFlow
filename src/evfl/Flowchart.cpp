#include <algorithm>
#include <evfl/Flowchart.h>
#include <evfl/Param.h>
#include <evfl/ResFlowchart.h>
#include <ore/Array.h>
#include <ore/BitUtils.h>
#include <ore/IterRange.h>
#include <ore/ResDic.h>
#include <ore/StringView.h>

namespace evfl {

namespace {

void RegisterBindings(FlowchartObj* obj, ore::BitArray* visited_events, int event_idx) {
    const ResActor* actors = obj->GetFlowchart()->actors.Get();
    const ResEvent* events = obj->GetFlowchart()->events.Get();

    while (event_idx != 0xffff && !visited_events->Test(event_idx)) {
        visited_events->Set(event_idx);
        const ResEvent& event = events[event_idx];
        const auto event_type = event.type;

        switch (event_type.mValue) {
        case ResEvent::EventType::kAction:
            if (actors[event.actor_idx].argument_name.Get()->empty()) {
                auto* action = actors[event.actor_idx].actions.Get() + event.actor_action_idx;
                auto& binding = obj->GetActBinder().GetBindings()[event.actor_idx];
                if (!binding.IsUsed() && binding.GetActor()->argument_name.Get()->empty()) {
                    obj->GetActBinder().IncrementNumActors();
                    binding.SetIsUsed(true);
                }
                binding.Register(action);
            }
            break;
        case ResEvent::EventType::kSwitch:
            if (actors[event.actor_idx].argument_name.Get()->empty()) {
                auto* query = actors[event.actor_idx].queries.Get() + event.actor_query_idx;
                auto& binding = obj->GetActBinder().GetBindings()[event.actor_idx];
                if (!binding.IsUsed() && binding.GetActor()->argument_name.Get()->empty()) {
                    obj->GetActBinder().IncrementNumActors();
                    binding.SetIsUsed(true);
                }
                binding.Register(query);
            }
            break;
        default:
            break;
        }

        // Process the next event.

        switch (event_type) {
        case ResEvent::EventType::kAction:
        case ResEvent::EventType::kJoin:
        case ResEvent::EventType::kSubFlow:
            event_idx = event.next_event_idx;
            break;
        case ResEvent::EventType::kSwitch: {
            ore::Array<const ResCase> cases{event.cases.Get(), event.num_cases};
            std::for_each(cases.begin(), cases.end(), [&](const ResCase& case_) {
                RegisterBindings(obj, visited_events, case_.event_idx);
            });
            return;
        }
        case ResEvent::EventType::kFork: {
            ore::Array<const u16> forks{event.fork_event_indices.Get(), event.num_forks};
            std::for_each(forks.begin(), forks.end(),
                          [&](u16 fork) { RegisterBindings(obj, visited_events, fork); });
            event_idx = event.join_event_idx;
            break;
        }
        }
    }
}

struct ActorArgumentInfo {
    bool operator==(const ActorArgumentInfo& rhs) const {
        return entry_point_idx == rhs.entry_point_idx && flowchart == rhs.flowchart &&
               rhs.GetActorArgumentName() == GetActorArgumentName();
    }
    bool operator!=(const ActorArgumentInfo& rhs) const { return !(*this == rhs); }

    ore::StringView GetActorArgumentName() const {
        return ore::StringView(actor_argument_name, actor_argument_name_len);
    }

    const ResFlowchart* flowchart;
    int entry_point_idx;
    const char* actor_argument_name;
    size_t actor_argument_name_len;
};

// NON_MATCHING: the if checks are reordered to hell for some reason
const ResActor* FindActor(const ActorArgumentInfo& entry) {
    ore::Array<const ResActor> actors{entry.flowchart->actors.Get(), entry.flowchart->num_actors};
    for (const auto& actor : actors) {
        if (actor.entry_point_idx != entry.entry_point_idx)
            continue;
        if (actor.argument_name.Get()->empty())
            continue;
        if (*actor.argument_name.Get() != entry.GetActorArgumentName())
            continue;
        return &actor;
    }
    return nullptr;
}

void RegisterBindingsForArguments(ActBinder& binder, int actor_idx,
                                  const ore::IterRange<const ResFlowchart* const*>& flowcharts,
                                  ore::ArrayListBase<ActorArgumentInfo>& processed,
                                  const ActorArgumentInfo& entry) {
    if (std::find(processed.begin(), processed.end(), entry) != processed.end())
        return;

    processed.emplace_back(entry);

    if (auto* actor = FindActor(entry)) {
        ore::Array<const ResAction> actions{actor->actions.Get(), actor->num_actions};
        for (const auto& action : actions) {
            auto& binding = binder.GetBindings()[actor_idx];
            if (!binding.IsUsed() && binding.GetActor()->argument_name.Get()->empty()) {
                binder.IncrementNumActors();
                binding.SetIsUsed(true);
            }
            binding.Register(&action);
        }

        ore::Array<const ResQuery> queries{actor->queries.Get(), actor->num_queries};
        for (const auto& query : queries) {
            auto& binding = binder.GetBindings()[actor_idx];
            if (!binding.IsUsed() && binding.GetActor()->argument_name.Get()->empty()) {
                binder.IncrementNumActors();
                binding.SetIsUsed(true);
            }
            binding.Register(&query);
        }
    }

    const auto& entry_point = entry.flowchart->entry_points.Get()[entry.entry_point_idx];
    ore::Array<const u16> sub_flow_event_indices{entry_point.sub_flow_event_indices.Get(),
                                                 entry_point.num_sub_flow_event_indices};
    for (auto sub_flow_event_idx : sub_flow_event_indices) {
        const auto& event = entry.flowchart->events.Get()[sub_flow_event_idx];
        const ore::StringView sub_flow_flowchart = *event.sub_flow_flowchart.Get();
        const ore::StringView sub_flow_entry_point = *event.sub_flow_entry_point.Get();

        auto* params = event.params.Get();
        if (!params)
            continue;

        for (int i = 0; i < params->num_items; ++i) {
            auto* param = (&params->value.container + i)->Get();
            if (param->type != ore::ResMetaData::DataType::kArgument)
                continue;
            if (entry.GetActorArgumentName() != *param->value.str.Get())
                continue;

            const ResFlowchart* flowchart = entry.flowchart;
            if (!sub_flow_flowchart.empty()) {
                flowchart =
                    *std::find_if(flowcharts.begin(), flowcharts.end(), [=](const ResFlowchart* f) {
                        return sub_flow_flowchart == *f->name.Get();
                    });
            }

            const auto entry_point_idx =
                flowchart->entry_point_names.Get()->FindIndex(sub_flow_entry_point);
            const auto actor_argument_name = params->dictionary.Get()->GetEntries()[1 + i].GetKey();

            ActorArgumentInfo arg;
            arg.flowchart = flowchart;
            arg.entry_point_idx = entry_point_idx;
            arg.actor_argument_name = actor_argument_name.data();
            arg.actor_argument_name_len = actor_argument_name.size();
            RegisterBindingsForArguments(binder, actor_idx, flowcharts, processed, arg);
        }
    }

    processed.pop_back();
}

void RegisterBindingsForActorIdentifiers(FlowchartObj* obj,
                                         ore::IterRange<const ResFlowchart* const*> flowcharts,
                                         int entry_point_idx) {
    auto* flowchart = obj->GetFlowchart();
    const auto& entry_point = flowchart->entry_points.Get()[entry_point_idx];

    ore::IterRange<const u16*> sub_flow_event_indices{entry_point.sub_flow_event_indices.Get(),
                                                      entry_point.num_sub_flow_event_indices};
    const ResEvent* events = flowchart->events.Get();
    ore::IterRange<const ResActor*> actors{flowchart->actors.Get(), flowchart->num_actors};

    for (auto sub_flow_event_idx : sub_flow_event_indices) {
        const auto& event = events[sub_flow_event_idx];

        auto* params = event.params.Get();
        if (!params)
            continue;

        const ore::StringView sub_flow_flowchart = *event.sub_flow_flowchart.Get();
        const ore::StringView sub_flow_entry_point = *event.sub_flow_entry_point.Get();
        const ResFlowchart* arg_flowchart = flowchart;
        if (!sub_flow_flowchart.empty()) {
            arg_flowchart =
                *std::find_if(flowcharts.begin(), flowcharts.end(), [=](const ResFlowchart* f) {
                    return sub_flow_flowchart == *f->name.Get();
                });
        }

        for (int i = 0; i < params->num_items; ++i) {
            auto* param = (&params->value.container + i)->Get();
            if (param->type != ore::ResMetaData::DataType::kActorIdentifier)
                continue;

            const ore::StringView param_name =
                params->dictionary.Get()->GetEntries()[1 + i].GetKey();
            const ore::StringView name = *param->value.actor.name.Get();
            const ore::StringView sub_name = *param->value.actor.sub_name.Get();
            auto* actor = std::find_if(actors.begin(), actors.end(), [&](const ResActor& a) {
                return name == *a.name.Get() && sub_name == *a.secondary_name.Get();
            });
            const int actor_idx = std::distance(actors.begin(), actor);
            const int arg_entry_point_idx =
                arg_flowchart->entry_point_names.Get()->FindIndex(sub_flow_entry_point);

            ore::FixedArrayList<ActorArgumentInfo, 64> processed;
            ActorArgumentInfo arg;
            arg.flowchart = arg_flowchart;
            arg.entry_point_idx = arg_entry_point_idx;
            arg.actor_argument_name = param_name.data();
            arg.actor_argument_name_len = param_name.size();
            RegisterBindingsForArguments(obj->GetActBinder(), actor_idx, flowcharts, processed,
                                         arg);
        }
    }
}

}  // namespace

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
        handler->_18 = -1;
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

bool FlowchartObj::Builder::Build(FlowchartObj* obj, ore::Allocator* allocator,
                                  ore::IterRange<const ResFlowchart* const*> flowcharts) {
    obj->m_flowchart = m_flowchart;
    m_act_binder_builder.Build(&obj->GetActBinder(), allocator,
                               {m_flowchart->actors.Get(), m_flowchart->num_actors});

    const int num_events = m_flowchart->num_events;
    ore::BitArray visited_events{allocator, num_events};

    auto* entry_points = obj->m_flowchart->entry_points.Get();
    auto entry_point_it = m_entry_points_mask->BeginTest();
    const auto entry_point_end = m_entry_points_mask->EndTest();
    while (entry_point_it != entry_point_end) {
        RegisterBindings(obj, &visited_events, entry_points[*entry_point_it].main_event_idx);
        RegisterBindingsForActorIdentifiers(obj, flowcharts, *entry_point_it);
        obj->GetActBinder().SetIsUsed();
        ++entry_point_it;
    }

    visited_events.FreeBuffer(allocator);
    return true;
}

}  // namespace evfl
