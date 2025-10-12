#include <amethyst/Imports.hpp>
#include <mc/src-deps/core/string/StringHash.hpp>

class AttributeInstanceHandle {};

class Attribute {
public:
	char mRedefinitionMode;
	bool mSyncable;
	int mId;
	HashedString mName;
};

class AttributeInstance {
private:
	char pad[132];
public:
	float mCurrentValue;

public:
	float getCurrentValue() {
		return *(float*)(this + 132);
	}
};

class BaseAttributeMap {
public:
	std::unordered_map<int, AttributeInstance> mInstanceMap;
	std::vector<AttributeInstanceHandle> mDirtyAttributes;

public:
	AttributeInstance& getAttribute(int id) {
		auto attribute = mInstanceMap.find(id);
		if (attribute == mInstanceMap.end())
			Log::Info("Couldn't find attribute with id {}", id);
		return attribute->second;
	}
};

struct AttributesComponent {
public:
	BaseAttributeMap mAttributes;
};

static_assert(sizeof(AttributesComponent) == 88);