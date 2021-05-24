#pragma once

#include <ore/BinaryFile.h>

namespace ore {
struct ResEndian;
struct ResMetaData;
}  // namespace ore

namespace evfl {

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

void SwapEndian(ore::ResEndian* endian, ResActor* actor);

}  // namespace evfl
