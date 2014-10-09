#include "req.h"

#include "CRC16.h"


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RequestQuery::Add(REQ* req)
{
	if (req != 0)
	{
		if (_first == 0)
		{
			_first = _last = req;
		}
		else
		{
			_last->next = req;
			_last = req;
		};

		req->next = 0;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* RequestQuery::Get()
{
	REQ* r = _first;

	if (_first != 0)
	{
		_first = _first->next;
	};

	return r;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RequestQuery::Update()
{
	static byte i = 0;
	static REQ* req = 0;

		switch(i)
		{
			case 0:

				if (!Empty())
				{
					req = Get();
					i++;
				};

				break;

			case 1:

				if (req != 0 && req->wb != 0)
				{
					DataPointer p(req->wb->data);
					p.b += req->wb->len;
					*p.w = GetCRC16(req->wb->data, req->wb->len);
					req->wb->len += 2;
					com->Write(req->wb);
					i++;
				}
				else
				{
					i = 0;
				};

				break;

			case 2:

				if (!com->Update())
				{
					if (req->rb != 0)
					{
						com->Read(req->rb, req->preTimeOut, req->postTimeOut); 
						i++;
					}
					else
					{
						i += 2;
					};
				};

				break;

			case 3:

				if (!com->Update())
				{
					req->crcOK = GetCRC16(req->rb->data, req->rb->len) == 0;
					i++;
				};

				break;

			case 4:

				if (req->CallBack != 0)
				{
					req->CallBack(req);
				};

				i = 0;

				break;
		};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
