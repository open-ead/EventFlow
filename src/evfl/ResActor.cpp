#include <evfl/ResActor.h>
#include <ore/ResEndian.h>
#include <ore/ResMetaData.h>

namespace evfl {

void SwapEndian(ore::ResEndian* endian, ResActor* actor) {
    using ore::SwapEndian;
    if (endian->is_serializing) {
        if (auto* params = actor->params.ToPtr(endian->base))
            SwapEndian(endian, params);
        SwapEndian(&actor->num_actions);
        SwapEndian(&actor->num_queries);
        SwapEndian(&actor->entry_point_idx);
    } else {
        SwapEndian(&actor->num_actions);
        SwapEndian(&actor->num_queries);
        SwapEndian(&actor->entry_point_idx);
        if (auto* params = actor->params.ToPtr(endian->base))
            SwapEndian(endian, params);
    }
}

}  // namespace evfl
