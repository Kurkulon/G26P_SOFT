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
		r->next = 0;
	};

	return r;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RequestQuery::Update()
{
	static byte i = 0;

		switch(i)
		{
			case 0:

				if ((_req = Get()) != 0) i++;

				break;

			case 1:

				if (_req->wb != 0)
				{
					DataPointer p(_req->wb->data);
					p.b += _req->wb->len;
					*p.w = GetCRC16(_req->wb->data, _req->wb->len);
					_req->wb->len += 2;
					com->Write(_req->wb);
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
					if (_req->rb != 0)
					{
						com->Read(_req->rb, _req->preTimeOut, _req->postTimeOut); 
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
//					_req->crcOK = GetCRC16(_req->rb->data, _req->rb->len) == 0;
					i++;
				};

				break;

			case 4:

				if (_req->CallBack != 0)
				{
					_req->CallBack(_req);
				};

				i = 0;

				break;
		};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
