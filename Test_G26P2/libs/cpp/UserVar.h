#ifndef USERVAR_H__04_04_2008__10_51
#define USERVAR_H__04_04_2008__10_51

#pragma once

class UserVar;

#include "types.h"
#include "PointerCRC.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class UserVar
{
  protected:

	const word _code;

  public:

	UserVar(word ce) : _code(ce) {}
	UserVar() : _code(0) {}

	virtual float	MaxF() const = 0;
	virtual long	MaxL() const = 0;
	virtual float	MinF() const = 0;
	virtual long	MinL() const = 0;

	virtual void	SetLong(long v) = 0;
	virtual void	SetFloat(float v) = 0;

	virtual long	GetL() const = 0;
	virtual float	GetF() const = 0;

	virtual void	ResetToDefault() = 0;
	virtual void	Save(PointerCRC &p) const = 0;
	virtual void	Load(PointerCRC &p) = 0;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> class User : public UserVar
{
  public:

	typedef	bool (*Event)(word c, T prev, T cur);

	T v;

  protected:

	const T _min, _max, _def;

	Event const _ChangeEvent;

	static bool DefaultChangeEvent(word c, T prev, T cur);

  public:

	User(T a = 0) : v(a), _min(0), _max(0), _def(0), _ChangeEvent(DefaultChangeEvent) {}
	User(T a, T min, T max, T def, Event chev, word ce) 
		: UserVar(ce), v(a), _min(min), _max(max), _def(def), _ChangeEvent((chev!=0) ? chev : DefaultChangeEvent) {}

	operator T() const { return v; }
	void operator=(T a){ v = a; }
	void SetValue(T v);

	virtual float	MaxF() const;
	virtual long	MaxL() const;
	virtual float	MinF() const;
	virtual long	MinL() const;

	virtual void	SetLong(long v);
	virtual void	SetFloat(float v);
	
	virtual long	GetL() const;
	virtual float	GetF() const;

	virtual void	ResetToDefault();
	virtual void	Save(PointerCRC &p) const;
	virtual void	Load(PointerCRC &p);
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> bool User<T>::DefaultChangeEvent(word c, T prev, T cur)
{
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> float User<T>::MaxF() const
{
	return _max;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> long User<T>::MaxL() const
{
	return _max;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> float User<T>::MinF() const
{
	return _min;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> long User<T>::MinL() const
{
	return _min;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> void	 User<T>::SetValue(T a)
{
	if (a < _min)
	{
		a = _min;
	}
	else if (a > _max)
	{
		a = _max;
	};

	if (v != a)
	{
		T temp = v;
		v = a;

		if (_ChangeEvent != 0) _ChangeEvent(_code, temp, v);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> void	 User<T>::SetLong(long a)
{
	if (a < _min)
	{
		a = _min;
	}
	else if (a > _max)
	{
		a = _max;
	};

	if (v != a)
	{
		T temp = v;
		v = (T)a;

		if (_ChangeEvent != 0) _ChangeEvent(_code, temp, v);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> void	 User<T>::SetFloat(float a)
{
	if (a < _min)
	{
		a = _min;
	}
	else if (a > _max)
	{
		a = _max;
	};

	if (v != a)
	{
		T temp = v;
		v = (T)a;

		if (_ChangeEvent != 0) _ChangeEvent(_code, temp, v);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> long User<T>::GetL() const
{
	return v;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> float User<T>::GetF() const
{
	return v;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> void	 User<T>::ResetToDefault()
{
	v = _def;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> void	 User<T>::Save(PointerCRC &p) const
{
	p.WriteArrayB(&v, sizeof(T));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> void	 User<T>::Load(PointerCRC &p)
{
	p.ReadArrayB(&v, sizeof(T));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



#endif // USERVAR_H__04_04_2008__10_51
