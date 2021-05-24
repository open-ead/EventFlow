#pragma once

#include <ore/Types.h>

namespace ore {
struct ResMetaData;
}

namespace evfl {

class FlowchartContext;

// TODO
class ParamAccessor {
public:
    explicit ParamAccessor(const ore::ResMetaData*);
    explicit ParamAccessor(const FlowchartContext*, int);

private:
    ore::ResMetaData* mResMetaData;
    FlowchartContext* mFlowchartCtx;
    int mNodeIdx;
    u32 mNodeCounter;
};

}  // namespace evfl
