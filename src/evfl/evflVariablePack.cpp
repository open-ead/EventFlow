#include <evfl/variable.h>

namespace evfl {
bool VariablePack::FindInt(s32* out_ptr, const ore::TStringView& a) const{
	return false;
}
VariableType VariablePack::GetVariableType(const ore::TStringView& name) const {
	s32 found = m_res_dic->Find(name);
	if(found==-1) return VariableType::Type13;
	found = m_res_dic->Find(name);
	return m_array[found].type;
}
//Use this to match Find()
VariablePack* VariablePack::SetInt(const ore::TStringView& name, s32 val){
	m_array[m_res_dic->Find(name)].value = reinterpret_cast<void*>(val);
	return this;
}

}