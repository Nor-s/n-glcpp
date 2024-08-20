#ifndef ANIM_ENTITY_COMPONENT_COMPONENT_H
#define ANIM_ENTITY_COMPONENT_COMPONENT_H

#include <memory>
#include <vector>

namespace anim
{
class Entity;

typedef int* TypeID;
class Component
{
	friend class Entity;

public:
	virtual ~Component() = default;
	virtual TypeID get_type() const = 0;
	virtual void pre_update()
	{
	}
	virtual void update() = 0;
	virtual void post_update()
	{
	}
	Entity* get_owner() const
	{
		return owner_;
	}

private:
	void set_owner(Entity* owner)
	{
		owner_ = owner;
	}

private:
	Entity* owner_{nullptr};
};

template <class T>
class ComponentBase : public Component
{
public:
	virtual ~ComponentBase() = default;
	virtual void update() override{};
	static TypeID type;
	TypeID get_type() const override
	{
		return T::type;
	}
};
template <typename T>
TypeID ComponentBase<T>::type((TypeID) &T::type);
}	 // namespace anim
#endif