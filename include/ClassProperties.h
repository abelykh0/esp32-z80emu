#ifndef __CLASSPROPERTIES_INCLUDED__
#define __CLASSPROPERTIES_INCLUDED__

// Abstraction for z80 Emulator

// Magic from https://www.codeproject.com/Articles/12358/C-object-properties-with-no-run-time-or-memory-ove.
template<class T, class V, V (T::*_get)(), void (T::*_set)(V)>
struct property
{
private:
	T* _this;
	V _value;
public:
	property(T* this_):_this(this_) { }
	operator V() { return (_this->*_get)(); }
	void operator=(V i) { (_this->*_set)(i);}
};
#define CLASS(NAME) typedef NAME ClassType
#define PROPERTY(TYPE, NAME) \
    property<ClassType, TYPE, &ClassType::get_##NAME, &ClassType::set_##NAME> NAME

#endif

