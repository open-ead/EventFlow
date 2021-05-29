#include <algorithm>
#include <evfl/Param.h>
#include <evfl/ResFlowchart.h>
#include <ore/ResDic.h>

namespace evfl {

VariablePack::VariableType VariablePack::GetVariableType(const ore::StringView& name) const {
    if (!Contains(name))
        return ore::ResMetaData::DataType::Invalid();
    return GetVariableEntry(name)->type;
}

bool VariablePack::FindInt(int* value, const ore::StringView& name) const {
    if (GetVariableType(name) != ore::ResMetaData::DataType::kInt)
        return false;
    *value = GetVariableEntry(name)->value.i;
    return true;
}

bool VariablePack::FindBool(bool* value, const ore::StringView& name) const {
    if (GetVariableType(name) != ore::ResMetaData::DataType::kBool)
        return false;
    *value = GetVariableEntry(name)->value.i;
    return true;
}

bool VariablePack::FindFloat(float* value, const ore::StringView& name) const {
    if (GetVariableType(name) != ore::ResMetaData::DataType::kFloat)
        return false;
    *value = GetVariableEntry(name)->value.f;
    return true;
}

bool VariablePack::FindIntList(ore::DynArrayList<int>** value, const ore::StringView& name) const {
    if (GetVariableType(name) != ore::ResMetaData::DataType::kIntArray)
        return false;
    *value = GetVariableEntry(name)->value.int_array;
    return true;
}

bool VariablePack::FindFloatList(ore::DynArrayList<float>** value,
                                 const ore::StringView& name) const {
    if (GetVariableType(name) != ore::ResMetaData::DataType::kFloatArray)
        return false;
    *value = GetVariableEntry(name)->value.float_array;
    return true;
}

bool VariablePack::Contains(const ore::StringView& name) const {
    return m_names->FindIndex(name) != -1;
}

VariablePack::VariablePack() = default;

VariablePack::~VariablePack() {
    Dispose();
}

template <typename T>
static void DestroyVariableArray(ore::Allocator* allocator, T* ptr) {
    if (*ptr == nullptr)
        return;
    std::destroy_at(*ptr);
    auto* array = *ptr;
    allocator->FreeImpl(array);
    *ptr = nullptr;
}

void VariablePack::Dispose() {
    auto* variables = m_variables.data();
    if (!variables)
        return;

    for (auto& variable : m_variables) {
        switch (variable.type) {
        case ore::ResMetaData::DataType::kIntArray:
            DestroyVariableArray(GetAllocator(), &variable.value.int_array);
            break;
        case ore::ResMetaData::DataType::kFloatArray:
            DestroyVariableArray(GetAllocator(), &variable.value.float_array);
            break;
        default:
            break;
        }
    }
    GetAllocator()->FreeImpl(variables);
}

void VariablePack::Init(AllocateArg arg, const ResEntryPoint* entry_point) {
    Dispose();
    m_names = entry_point->variable_defs_names.Get();
    m_allocator = evfl::EvflAllocator{arg};
    m_variables.AllocateElements(m_names->num_entries, GetAllocator());

    const ResVariableDef* def = entry_point->variable_defs.Get();
    for (auto& variable : m_variables) {
        variable.type = def->type;
        variable.value = {};
        switch (variable.type) {
        case ore::ResMetaData::DataType::kArgument:
        case ore::ResMetaData::DataType::kContainer:
            break;
        case ore::ResMetaData::DataType::kInt:
        case ore::ResMetaData::DataType::kBool:
            variable.value.i = def->value.i;
            break;
        case ore::ResMetaData::DataType::kFloat:
            variable.value.f = def->value.f;
            break;
        case ore::ResMetaData::DataType::kString:
        case ore::ResMetaData::DataType::kWString:
            break;
        case ore::ResMetaData::DataType::kIntArray: {
            variable.value.int_array = GetAllocator()->New<ore::DynArrayList<int>>();
            variable.value.int_array->Init(GetAllocator(), 2);
            ore::Array<const int> values{def->value.int_array.Get(), def->num};
            variable.value.int_array->OverwriteWith(values.begin(), values.end());
            break;
        }
        case ore::ResMetaData::DataType::kBoolArray:
            break;
        case ore::ResMetaData::DataType::kFloatArray: {
            variable.value.float_array = GetAllocator()->New<ore::DynArrayList<float>>();
            variable.value.float_array->Init(GetAllocator(), 2);
            ore::Array<const float> values{def->value.float_array.Get(), def->num};
            variable.value.float_array->OverwriteWith(values.begin(), values.end());
            break;
        }
        case ore::ResMetaData::DataType::kStringArray:
        case ore::ResMetaData::DataType::kWStringArray:
        case ore::ResMetaData::DataType::kActorIdentifier:
            break;
        }
        ++def;
    }
}

const VariablePack::Entry* VariablePack::GetVariableEntry(const ore::StringView& name) const {
    return &m_variables[m_names->FindIndex(name)];
}

ore::StringView VariablePack::GetVariableName(int idx) const {
    return *m_names->GetEntries()[1 + idx].name.Get();
}

int VariablePack::GetVariableCount() const {
    return m_names->num_entries;
}

VariablePack::Entry* VariablePack::GetVariableEntry(const ore::StringView& name) {
    return &m_variables[m_names->FindIndex(name)];
}

void VariablePack::SetInt(const ore::StringView& name, int value) {
    m_variables[m_names->FindIndex(name)].value.i = value;
}

void VariablePack::SetFloat(const ore::StringView& name, float value) {
    m_variables[m_names->FindIndex(name)].value.f = value;
}

void VariablePack::SetBool(const ore::StringView& name, bool value) {
    m_variables[m_names->FindIndex(name)].value.i = value;
}

int VariablePack::GetInt(const ore::StringView& name) const {
    int value;
    FindInt(&value, name);
    return value;
}

bool VariablePack::GetBool(const ore::StringView& name) const {
    bool value;
    FindBool(&value, name);
    return value;
}

float VariablePack::GetFloat(const ore::StringView& name) const {
    float value;
    FindFloat(&value, name);
    return value;
}

ore::DynArrayList<int>* VariablePack::GetIntList(const ore::StringView& name) const {
    ore::DynArrayList<int>* value;
    FindIntList(&value, name);
    return value;
}

ore::DynArrayList<float>* VariablePack::GetFloatList(const ore::StringView& name) const {
    ore::DynArrayList<float>* value;
    FindFloatList(&value, name);
    return value;
}

}  // namespace evfl
