#pragma once
#include <amethyst/Imports.hpp>
#include <mc/src-deps/core/string/StringHash.hpp>

/*
* https://github.com/LiteLDev/LeviLamina/blob/1046f89f8f46ff854b9c3500b4fdadb4504e4570/src-server/mc/world/attribute/BaseAttributeMap.h
* https://github.com/LiteLDev/LeviLamina/blob/1046f89f8f46ff854b9c3500b4fdadb4504e4570/src-server/mc/entity/components/AttributesComponent.h
*/

class AttributeInstanceHandle {};

class AttributeInstance {
private:
	char pad[132];
public:
	float mCurrentValue;
};

class BaseAttributeMap {
public:
	std::unordered_map<int, AttributeInstance> mInstanceMap;
	std::vector<AttributeInstanceHandle> mDirtyAttributes;

public:
	AttributeInstance& getInstance(unsigned int id) {
		using function = AttributeInstance&(*)(BaseAttributeMap*, unsigned int);
		static auto func = std::bit_cast<function>(SigScan("4C 8B C9 89 54 24 ? 48 B9 ? ? ? ? ? ? ? ? 44 0F B6 C2 48 B8 ? ? ? ? ? ? ? ? 4C 33 C0 0F B6 44 24 ? 4C 0F AF C1 4C 33 C0 0F B6 44 24 ? 4C 0F AF C1 4C 33 C0 0F B6 44 24 ? 4C 0F AF C1 4C 33 C0 4C 0F AF C1 49 8B 49 ? 49 23 C8 4D 8B 41 ? 48 C1 E1 ? 49 03 49 ? 48 8B 41 ? 49 3B C0 74 ? 48 8B 09 3B 50 ? 74 ? 66 90 48 3B C1 74 ? 48 8B 40 ? 3B 50 ? 75 ? EB ? 33 C0 48 85 C0 48 8D 15"));
		return func(this, id);
	}
};

struct AttributesComponent {
public:
	BaseAttributeMap mAttributes;
};

static_assert(sizeof(AttributesComponent) == 88);