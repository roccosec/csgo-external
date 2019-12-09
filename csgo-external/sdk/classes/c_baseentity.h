#pragma once

#include <stdint.h>
#include <misc/vector.h>
#include <misc/matrix.h>
#include <epic/read_write_variable.h>

#include "icliententity.h"

#include "defines.h"
#include "studio.h"


namespace sdk {
	// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/client/c_baseentity.h#L175
	class C_BaseEntity : public IClientEntity {
	public:
		C_BaseEntity() = default;
		explicit C_BaseEntity(const uint32_t address) noexcept;

		// get the underlying address
		uint32_t get_base_entity_addr() const { return this->m_base_entity_addr; }

	public:
		mango::ReadWriteVariable<bool> m_bSpotted;
		mango::ReadWriteVariable<int> m_iTeamNum;
		mango::ReadWriteVariable<mango::Vec3f> m_vecOrigin;

	private:
		uint32_t m_base_entity_addr = 0;
	};
} // namespace sdk