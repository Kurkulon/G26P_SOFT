#include "req.h"

#include "CRC16.h"


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RequestQuery::Add(REQ* req)
{
	if (req != 0)
	{
		req->ready = false;

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

		count++;
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

		count--;
	};

	return r;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RequestQuery::Update()
{
		switch(_state)
		{
			case 0:

				_req = (_run) ? Get() : 0;

				if (_req != 0) _state++;

				break;

			case 1:

				if (_req->wb != 0)
				{
					if (_req->updateCRC)
					{
						_crc = 0xFFFF;
						_crcLen = _req->wb->len;
						_crcPtr = (byte*) _req->wb->data;

						_state++;
					}
					else
					{
						com->Write(_req->wb);
						_state++;
					};
				}
				else
				{
					_state = 0;
				}; 

				break;

			case 2:

				{
					u16 len = 100;

					if (_crcLen < len) len = _crcLen;

					_crcLen -= len;

					HW::PIOB->SODR = 1<<10;

					_crc = GetCRC16(_crcPtr, len, _crc, 0);

					HW::PIOB->CODR = 1<<10;

					_crcPtr += len;

					if (_crcLen == 0)
					{
						DataPointer p(_crcPtr);
						*p.w = _crc;
						_req->wb->len += 2;
						_req->updateCRC = false;

						com->Write(_req->wb);
						_state++;
					};
				};

				break;

			case 3:

				if (!com->Update())
				{
					if (_req->rb != 0)
					{
						com->Read(_req->rb, _req->preTimeOut, _req->postTimeOut); 
						_state++;
					}
					else
					{
						_state = 6;
					};
				};

				break;

			case 4:

				if (!com->Update())
				{
					if (_req->checkCRC && _req->rb->recieved)
					{
						_crc = 0xFFFF;
						_crcLen = _req->rb->len;
						_crcPtr = (byte*) _req->rb->data;

						_state++;
					}
					else
					{
						_req->crcOK = false;
						_state = 6;
					};
				};

				break;

			case 5:

				{
					u16 len = 100;

					if (_crcLen < len) len = _crcLen;

					_crcLen -= len;

//					HW::PIOB->SODR = 1<<10;

					_crc = GetCRC16(_crcPtr, len, _crc, 0);

//					HW::PIOB->CODR = 1<<10;

					_crcPtr += len;

					if (_crcLen == 0)
					{
						_req->crcOK = _crc == 0;
						_state++;
					};
				};

				break;

			case 6:

				_req->ready = true;

				if (_req->CallBack != 0)
				{
					//if (!HW::RomCheck((void*)_req->CallBack))
					//{
					//	__breakpoint(0);
					//};

					_req->CallBack(_req);
				};


				_state = 0;

				break;

			default:

				_state = 0;
		};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
