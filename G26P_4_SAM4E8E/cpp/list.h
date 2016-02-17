#ifndef LIST_H__03_02_2016__14_38
#define LIST_H__03_02_2016__14_38

#include <types.h>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> struct List
{

protected:

	T *first;
	T *last;

  public:

	List() : first(0), last(0) {}

	T*		Get();
	void	Add(T* r);
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> T* List<T>::Get()
{
	T* r = first;

	if (r != 0)
	{
		first = (T*)r->next;

		if (first == 0)
		{
			last = 0;
		};

	};

	return r;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> void List<T>::Add(T* r)
{
	if (r == 0)
	{
		return;
	};

	if (last == 0)
	{
		first = last = r;
		r->next = 0;
	}
	else
	{
		last->next = r;
		last = r;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // LIST_H__03_02_2016__14_38