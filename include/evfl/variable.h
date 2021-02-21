#pragma once

#include <ore/string.h>
#include <ore/array.h>
#include <ore/res.h>
#include <evfl/types.h>

namespace evfl{

class AllocateArg;
class ResEntryPoint{
	u8 TEMP_0[0x20];
};
class EvflAllcator{
	u8 TEMP_0[0x28];
};

enum class VariableType : u32 {
	Type0 = 0,
	Type1 = 1,
	Int = 2,
	Type13 = 13,
};

struct VarPackEntry{
	void* value;
	VariableType type;
	u32 _c;
};

class VariablePack{
public:
	VariablePack();
	virtual ~VariablePack();
	
	//return type TBD
	bool FindInt(s32* out_ptr, const ore::TStringView& a) const;
	//return type TBD
	bool FindBool(bool* out_ptr, const ore::TStringView& a) const;
	//rtTBD
	bool FindFloat(f32* out_ptr, const ore::TStringView& a) const;
	//rtTBD
	bool FindIntList(ore::DynArrayList<s32>** out_ptr, const ore::TStringView& a) const;
	//rtTBD
	bool FindFloatList(ore::DynArrayList<f32>** out_ptr, const ore::TStringView& a) const;
	//rtTBD
	bool Contains(const ore::TStringView& a) const;
	//rtTBD
	void Dispose();
	//rtTBD
	void Init(AllocateArg arg, const ResEntryPoint* entry_point);
	VariableType GetVariableType(const ore::TStringView& a) const;
	//rtTBD
	void* GetVariableEntry(const ore::TStringView& a) const;
	//rtTBD
	void* GetVariableEntry(const ore::TStringView& a);
	//rtTBD
	void* GetVariableName(s32 i) const;
	//rtTBD
	void* GetVariableCount() const;
	//rtTBD
	void* GetInt(const ore::TStringView& a) const;
	//rtTBD
	void* GetBool(const ore::TStringView& a) const;
	//rtTBD
	void* GetFloat(const ore::TStringView& a) const;
	//rtTBD
	void* GetIntList(const ore::TStringView& a) const;
	//rtTBD
	void* GetFloatList(const ore::TStringView& a) const;
	VariablePack* SetInt(const ore::TStringView& name, s32 val);
	//rtTBD
	void SetBool(const ore::TStringView& a, bool val);
	//rtTBD
	void SetFloat(const ore::TStringView& a, f32 val);
	//rtTBD
	void* SetIntList(const ore::TStringView& a,ore::DynArrayList<s32>* val);//not in
	//rtTBD
	void* SetFloatList(const ore::TStringView& a,ore::DynArrayList<f32>* val);//not in

private:
	ore::ResDic* m_res_dic = nullptr;
	VarPackEntry* m_array = nullptr;
	//tTBD
	u32 mArraySize = 0;
	u32 _14;//gap
	//tTBD
	EvflAllcator mAllocator;

};

}