#pragma once

#include <evfl/EvflAllocator.h>
#include <ore/Array.h>
#include <ore/ResMetaData.h>
#include <ore/StringView.h>
#include <ore/Types.h>

namespace ore {
struct ResDic;
struct ResMetaData;
}  // namespace ore

namespace evfl {

class FlowchartContext;
struct ResEntryPoint;

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

class VariablePack {
public:
    using VariableType = ore::ResMetaData::DataType::Type;

    struct Entry {
        union Value {
            void* dummy;
            int i;
            float f;
            ore::DynArrayList<int>* int_array;
            ore::DynArrayList<float>* float_array;
        };

        Value value;
        VariableType type;
    };

    VariablePack();
    ~VariablePack();
    VariablePack(const VariablePack&) = delete;
    auto operator=(const VariablePack&) = delete;

    void Init(AllocateArg arg, const ResEntryPoint* entry_point);

    Entry* GetVariableEntry(const ore::StringView& name);
    const Entry* GetVariableEntry(const ore::StringView& name) const;
    VariableType GetVariableType(const ore::StringView& name) const;
    ore::StringView GetVariableName(int idx) const;
    int GetVariableCount() const;

    bool Contains(const ore::StringView& name) const;

    bool FindInt(int* value, const ore::StringView& name) const;
    bool FindBool(bool* value, const ore::StringView& name) const;
    bool FindFloat(float* value, const ore::StringView& name) const;
    bool FindIntList(ore::DynArrayList<int>** value, const ore::StringView& name) const;
    bool FindFloatList(ore::DynArrayList<float>** value, const ore::StringView& name) const;

    int GetInt(const ore::StringView& name) const;
    bool GetBool(const ore::StringView& name) const;
    float GetFloat(const ore::StringView& name) const;
    ore::DynArrayList<int>* GetIntList(const ore::StringView& name) const;
    ore::DynArrayList<float>* GetFloatList(const ore::StringView& name) const;

    void SetInt(const ore::StringView& name, int value);
    void SetFloat(const ore::StringView& name, float value);
    void SetBool(const ore::StringView& name, bool value);

private:
    void Dispose();
    ore::Allocator* GetAllocator() { return &m_allocator; }

    const ore::ResDic* m_names = nullptr;
    ore::Array<Entry> m_variables;
    EvflAllocator m_allocator;
};

}  // namespace evfl
