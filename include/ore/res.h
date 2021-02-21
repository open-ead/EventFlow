#pragma once

#include <ore/string.h>
#include <evfl/types.h>

namespace ore {

struct ResDicNode{
	s32 m_compact_bit_index;
	u16 m_next_idx[2];
	PascalString* m_str;
};

class ResDic{
public:
	ResDicNode* Entries() { return reinterpret_cast<ResDicNode*>(this + 1); }
	ResDicNode* GetEntry(u32 i) {return &m_root+i; }
	inline s32 Find(const TStringView& str){
		auto* found_node = FindNode(str);
		if(!str.Equals(*found_node->m_str)) return -1;
		//if(found_node - Entries()==-1) return -1;
		return found_node - Entries();
	}
	inline ResDicNode* FindNode(const TStringView& str){
		ResDicNode* current = GetEntry(0);
		u32 next_idx = current->m_next_idx[0];
		ResDicNode* next = GetEntry(next_idx);
		
		while(current->m_compact_bit_index < next->m_compact_bit_index){
			s32 bit_idx = next->m_compact_bit_index;
			u32 which_idx;
			if((u32)str.Len() <= (u32)(bit_idx>>3)){
				which_idx = 0;
			}else{
				which_idx = (str.Cstr()[str.Len() + ~(bit_idx>>3)] >> (bit_idx & 7)) & 1;
			}
			current = GetEntry(next_idx);
			next_idx = current->m_next_idx[which_idx];
			next = GetEntry(next_idx);
		}
		return next;
	}
private:
	char magic[4];
	u32 m_variable_count;
	ResDicNode m_root;
};

}