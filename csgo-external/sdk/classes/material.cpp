#include "material.h"

#include "../misc/constants.h"


namespace sdk {
	// material name
	std::string Material::get_name() const {
		// ref @GetName
		const auto vfunc = globals::process.get_vfunc<uint32_t>(this->m_address, 0);

		// add ecx, offset
		const auto offset = uint32_t(globals::process.read<uint8_t>(vfunc + 2));
		return this->get_name_imp(offset);
	}

	// material texture group name
	std::string Material::get_texture_group_name() const {
		// ref @GetTextureGroupName
		const auto vfunc = globals::process.get_vfunc<uint32_t>(this->m_address, 1);

		// add ecx, offset
		const auto offset = uint32_t(globals::process.read<uint8_t>(vfunc + 2));
		return this->get_name_imp(offset);
	}

	// set the alpha of the material
	void Material::alpha_modulate(const float value) const {
		// mov     al, [esi+20h]
		// shr     al, 2
		// test    al, 1
		if ((globals::process.read<uint8_t>(this->m_address + 0x20) >> 2) == 1)
			return;
		
		// cmp     byte ptr [esi+22h], 5
		if (globals::process.read<uint8_t>(this->m_address + 0x22) <= 5)
			return;

		const auto materialvars = this->get_shader_params();
		if (!materialvars)
			return;
		
		// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/materialsystem/imaterialvar.h
		const auto alphavar = globals::process.read<uint32_t>(materialvars + 0x14);
		if (!alphavar)
			return;
		
		// ref @SetFloatValue
		globals::process.write<float>(alphavar + 0xC, value); // m_VecVal[0]
	}

	// an array of IMaterialVar*
	uint32_t Material::get_shader_params() const {
		// ref @GetShaderParams
		uint16_t func_bytes = globals::process.read<uint16_t>(globals::process.get_vfunc<uint32_t>(
			this->m_address, indices::get_shader_params));

		// black magic type shit
		if (func_bytes == 0x418B) {
			// 8B 41 24		mov     eax, [ecx+24h]
			return globals::process.read<uint32_t>(this->m_address + 0x24);
		} else if (func_bytes == 0x498B) {
			// mov     ecx, [ecx+4]
			return Material(globals::process.read<uint32_t>(this->m_address + 0x4)).get_shader_params();
		}

		return 0;
	}

	// i don't fully understand their mechanism for this, but basically a call to GetName()
	// or GetTextureGroupName() just adds a constant value to ecx (thisptr) then calls a function
	// which will then get the name/texture group name. weird.
	std::string Material::get_name_imp(const uint32_t offset) const {
		// mov     eax, someInstance
		// mov     ecx, eax
		// call    getName
		auto edi = globals::process.read<uint32_t>(globals::process.get_module_addr("materialsystem.dll") + 0xF9FF8);
		auto eax = uint32_t(globals::process.read<uint16_t>(this->m_address + offset));

		// has never happened for me
		if (eax == 0xFFFF)
			return "";

		// movzx   eax, ax
		// lea     ecx, [eax + eax * 2]
		// mov     eax, [edi + 4]
		// movzx   edx, word ptr[eax + ecx * 4 + 8]
		// lea     ecx, [eax + ecx * 4]
		// mov     eax, [edi + 24h]
		// movzx   ecx, word ptr[ecx + 0Ah]
		// mov     eax, [eax + edx * 4]
		// add     eax, 0Ah
		// add     eax, ecx
		auto ecx = eax * 3;
		eax = globals::process.read<uint32_t>(edi + 4);
		auto edx = uint32_t(globals::process.read<uint16_t>(eax + ecx * 4 + 8));
		ecx = eax + ecx * 4;
		eax = globals::process.read<uint32_t>(edi + 0x24);
		ecx = uint32_t(globals::process.read<uint16_t>(ecx + 0xA));
		eax = globals::process.read<uint32_t>(eax + edx * 4) + 0xA + ecx;

		// result is stored in eax
		char name_buffer[256];
		globals::process.read(eax, name_buffer, 256);
		name_buffer[255] = '\0';

		return name_buffer;
	}
} // namespace sdk