#pragma once

#include <ore/Array.h>
#include <ore/BinaryFile.h>
#include <ore/IterRange.h>

namespace ore {
struct ResEndian;
struct ResMetaData;
}  // namespace ore

namespace evfl {

struct ActionArg;
struct ActionDoneHandler;
struct QueryArg;

struct ResAction {
    ore::BinTPtr<ore::BinString> name;
};

struct ResQuery {
    ore::BinTPtr<ore::BinString> name;
};

struct ResActor {
    ore::BinTPtr<ore::BinString> name;
    ore::BinTPtr<ore::BinString> secondary_name;
    ore::BinTPtr<ore::BinString> argument_name;
    ore::BinTPtr<ResAction> actions;
    ore::BinTPtr<ResQuery> queries;
    ore::BinTPtr<ore::ResMetaData> params;
    u16 num_actions;
    u16 num_queries;
    /// Entry point index for assicated entry point (0xffff if none)
    u16 entry_point_idx;
    // TODO: Cut number? This is set to 1 for flowcharts but other values have been seen
    // for timeline actors.
    u8 cut_number;
};

using ActionHandler = void (*)(ActionArg* arg, ActionDoneHandler* done_handler);
using QueryHandler = void (*)(QueryArg* arg);

class ActorBinding {
public:
    struct Action {
        ActionHandler handler{};
        void* user_data{};
        const ore::BinTPtr<ore::BinString>* name{};
    };

    struct Query {
        QueryHandler handler{};
        void* user_data{};
        const ore::BinTPtr<ore::BinString>* name{};
    };

    ActorBinding() = default;

    void Register(const ResAction* action);
    void Register(const ResQuery* query);

    // Similar to std::find_if. Returns m_actions.end() if the specified action is not found.
    Action* GetAction(const ore::StringView& name);
    const Action* GetAction(const ore::StringView& name) const;
    Query* GetQuery(const ore::StringView& name);
    const Query* GetQuery(const ore::StringView& name) const;

    auto ActionsEnd() const { return m_actions.end(); }
    auto QueriesEnd() const { return m_queries.end(); }

    bool IsInitialized() const { return m_initialized; }
    bool IsIsUsed() const { return m_is_used; }

private:
    friend class ActBinder;

    ore::Vector<Action> m_actions{};
    ore::Vector<Query> m_queries{};
    void* m_user_data{};
    const ResActor* m_actor{};
    bool m_initialized{};
    bool m_is_used{};
};

class ActBinder {
public:
    class Builder {
    public:
        bool Build(evfl::ActBinder* binder, ore::Allocator* allocator,
                   ore::IterRange<const ResActor*> actors);
    };

    u32 GetEventUsedActorCount() const { return m_event_used_actor_count; }
    const ore::Array<ActorBinding>& GetUsedResActors() const;

private:
    u32 m_event_used_actor_count;
    ore::Allocator* m_allocator;
    ore::Array<ActorBinding> m_bindings;
    u32 _1c;
    u8 _20[8];
};

void SwapEndian(ore::ResEndian* endian, ResActor* actor);

}  // namespace evfl
