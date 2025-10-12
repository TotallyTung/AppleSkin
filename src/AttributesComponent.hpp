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
};

class BaseAttributeMap {
public:
	std::unordered_map<int, AttributeInstance> mInstanceMap;
	std::vector<AttributeInstanceHandle> mDirtyAttributes;
};

struct AttributesComponent {
public:
	BaseAttributeMap mAttributes;

public:
	AttributeInstance& getAttribute(int id) {
		auto attribute = mAttributes.mInstanceMap.find(id);
		if (attribute == mAttributes.mInstanceMap.end())
			Log::Info("Couldn't find attribute with id {}", id);
		return attribute->second;
	}
};

static_assert(sizeof(AttributesComponent) == 88);