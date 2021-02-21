#pragma once

#include <evfl/types.h>

namespace ore{
class TStringView{
public:
	TStringView(const char* data, const s32 len){
		m_cstr = data;
		m_len = len;
	}
	const char* Cstr() const {return m_cstr; }
	s32 Len() const {return m_len;}
	bool Equals(const TStringView& other) const{
		if (m_len!=other.m_len) return false;
		if(m_len>=1){
			s32 cmp_len = m_len+1;
			auto* this_ptr = m_cstr;
			auto* other_ptr = other.m_cstr;
			char this_c;
			char other_c;
			while(true){
				this_c = *this_ptr;
				other_c = *other_ptr;
				if(!*this_ptr || this_c!=other_c) break;
				--cmp_len;
				this_ptr++;
				other_ptr++;
				if(cmp_len<=1) return true;
			}
			if(this_c!=other_c) return false;
		}
		return true;
	}
private:
	const char* m_cstr;
	s32 m_len;

};
class PascalString{
public:
	const char* Data() const { return reinterpret_cast<const char*>(this + 1); }
	u16 m_len;
	operator TStringView() const { return TStringView(Data(), m_len); }
};
}