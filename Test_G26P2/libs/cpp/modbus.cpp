#ifdef WIN32

//#include <memory.h>
#include <winsock2.h>
#include <conio.h>
#include <stdio.h>

#endif

#include "modbus.h"
#include "time.h"


#ifdef _DEBUG_
//	static const bool _debug = true;
#else
//	static const bool _debug = false;
#endif


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Modbus::Modbus()

	: state(ReqState::WAIT), acsRights(AccessAll)
{
	connected = false;
	reqMessage.ready = false;
	rspMessage.ready = false;

	mode = 0;

	prevAccessTime = GetMilliseconds();
	accessTimeout = 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Modbus::Init(Vars& v)
{
	if (v.varsCount < 1)
	{
		while(1){};
		//printf("Modbus vars count = 0!!!\n");
		//exit(0);
	};

	this->vars = &v;

	maxEvents = 125;

	maxChains = 16;

	if (!CreateChains(vars))
	{
		while(1){};
		//printf("Error create chains!!!\n");
		//exit(0);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool Modbus::Connect(byte port, dword speed, byte parity, byte address, word delay)
{
	if (varsEvent == 0 || chains == 0 || connected || address == 0 || address > 127 || vars == 0)
	{
		return false;
	};

	state = ReqState::WAIT;
	mode = 0;

	time35 = US2RT(1000000*35/speed) + 1;

	time35 = (delay > time35) ? delay : time35;

	reqMessage.ready = false;
	rspMessage.ready = false;

	netAddress = address;
	reqMessage.ready = false;
	rspMessage.ready = false;

	diagnosticRegister = 0;
	listenOnlyMode = false;
	resetListenOnlyMode = false;
	busMessageCount = 0;
	busCommunicationErrorCount = 0;
	busExeptionErrorCount = 0;
	slaveMessageCount = 0;
	slaveNoResponseCount = 0;
	slaveCountNAK = 0;
	slaveBusyCount = 0;
	busCharacterOverrunCount = 0;

#ifdef WIN32
	printf("Modbus connect to COM%hu adr = %hu\r\n", port+1, address);
#endif

	return connected = comPort.Connect(port, speed, parity);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool Modbus::Disconnect()
{
	reqMessage.ready = false;
	rspMessage.ready = false;

	connected = !comPort.Disconnect();
	return !connected;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool Modbus::CreateChains(Vars* vars)
{
	int i, c = 0;
	word curAdr, curSize, prevAdr = vars->vars[0]->Address();

	chains[c].start = prevAdr;
	chains[c].count = 0;

	if (vars->varsCount < 1)
	{
		numChains = 0;
		return false;
	};

	for (i = 0; i < vars->varsCount; i++)
	{	
		curAdr = vars->vars[i]->Address();
		curSize = vars->vars[i]->Size();

		if (c >= 16)
		{
			// Ошибка: диппазоны не помещаются в массив
			while(1) {};
		};

		if (curAdr > prevAdr)
		{
			c += 1;
			chains[c].start = curAdr;
			chains[c].count = 0;
		}
		else if (curAdr < prevAdr)
		{
			// Ошибка: перекрывающиеся диаппазоны адресов
			while(1) {};
		};

		prevAdr = curAdr + curSize;
		chains[c].count += curSize;
	};

	numChains = c + 1;
	
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool Modbus::CheckChain(word start, word count)
{
	int l = 0, r = numChains - 1;
	int m; 
	word s,e;
	word end = start + count - 1;

	while (l < r)
	{
		m = (l + r) / 2;

		s = chains[m].start;
		e = chains[m].count + s - 1;

		if (e < start)
		{
			l = m + 1;
		}
		else
		{
			r = m;
		};
	};

	s = chains[l].start;
	e = chains[l].count + s - 1;

	return (start >= s && end <= e) ? true : false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Modbus::AddModbusEvent(word s, word c, byte* d, Var* v)
{
	varsEvent[numEvents].start = s;
	varsEvent[numEvents].count = c;
	varsEvent[numEvents].var = v;
	varsEvent[numEvents++].data = d;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Modbus::CreateEvents(word start, word count, byte *data)
{
	int ll = 0, lr = vars->varsCount - 1, rl = 0, rr = vars->varsCount - 1;
	int m; word t,s,c;
	word end = start + count - 1;

	while (ll < lr)
	{
		m = (ll + lr) / 2;// + ((ll+lr) & 1);

		t = vars->vars[m]->Address();
		t = vars->vars[m]->Size() + t - 1;

		if (t < start)
		{
			ll = m+1;
		}
		else
		{
			lr = m;
		};
	};

	while (rl < rr)
	{
		m = (rl + rr) / 2 + ((rl+rr) & 1);

		t = vars->vars[m]->Address();

		if (t > end)
		{
			rr = m-1;
		}
		else
		{
			rl = m;
		};
	};

	// ll - первая переменнаяя, rl - последняя переменная
	
	t = 0;

	if (ll == rl)
	{
		varsEvent[t].start = start;
		varsEvent[t].count = count;
		varsEvent[t].var = vars->vars[ll];
		varsEvent[t].data = data;

		t = 1;
	}
	else for (;ll <= rl ; ll++ )
	{
		s = vars->vars[ll]->Address();
		c = vars->vars[ll]->Size();

		if (s < start && s+c > start)
		{
			c -= start - s;
			s = start;
		}
		else if (s <= end && s+c-1 > end)
		{
			c = end - s + 1;
		};

		varsEvent[t].start = s;
		varsEvent[t].count = c;
		varsEvent[t].var = vars->vars[ll];
		varsEvent[t].data = data + (s-start)*2;
	
		t += 1;
	};

	numEvents = t;

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Modbus::CreateExeption(byte errorCode, byte exeptionCode, Message *response)
{
	response->len = 3;
	response->pdu.function = errorCode;
	response->pdu.data.b[0] = exeptionCode;
	//if (_debug) printf("CreateExeption %X, %X\n", (word)errorCode, (word)exeptionCode);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ReqState Modbus::ModbusFunction_03(const Message &request, Message *response)
{
	__packed struct Request		{ byte function; word startAddress; word count;	byte data[247]; };
	__packed struct Response	{ byte function; byte count; byte data[250]; };
	union { const PDU *reqPDU; const Request *req; };
	union { PDU *rspPDU; Response *rsp; };

	reqPDU = &request.pdu;
	rspPDU = &response->pdu;

	word start = ReverseWord(req->startAddress);
	word count = ReverseWord(req->count);

	if (count < 1 || count > 125 || request.len != 8)
	{
		CreateExeption(0x83, 0x03, response);
		return ReqState::RESPONSE;
	}
	else if (CheckChain(start, count))
	{
		CreateEvents(start, count, rsp->data);
		rsp->function = 0x03;
		rsp->count = (byte)(count * 2);
		response->len = count * 2 + 3;
	}
	else
	{
		CreateExeption(0x83, 0x02, response);
		return ReqState::RESPONSE;
	};
	
	return ReqState::READ;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ReqState Modbus::ModbusFunction_04(const Message &request, Message *response)
{
	__packed struct Request	{ byte function; word startAddress; word count;	byte data[247]; };
	__packed struct Response	{ byte function; byte count; byte data[250]; };
	union { const PDU *reqPDU; const Request *req; };
	union { PDU *rspPDU; Response *rsp; };

	reqPDU = &request.pdu;
	rspPDU = &response->pdu;

	word start = ReverseWord(req->startAddress);
	word count = ReverseWord(req->count);

	if (count < 1 || count > 125 || request.len != 8)
	{
		CreateExeption(0x84, 0x03, response);
		return ReqState::RESPONSE;
	}
	else if (CheckChain(start, count))
	{
		CreateEvents(start, count, rsp->data);
		rsp->function = 0x04;
		rsp->count = (byte)(count * 2);
		response->len = count * 2 + 3;
	}
	else
	{
		CreateExeption(0x84, 0x02, response);
		return ReqState::RESPONSE;
	};
	
	return ReqState::READ;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ReqState Modbus::ModbusFunction_06(const Message &request, Message *response)
{
	__packed struct Request	{ byte function; word startAddress; byte data[249]; };
	__packed struct Response	{ byte function; word startAddress; byte data[249]; };
	union { const PDU *reqPDU; Request *req; };
	union { PDU *rspPDU; Response *rsp; };

	reqPDU = &request.pdu;
	rspPDU = &response->pdu;

	word start = ReverseWord(req->startAddress);

	if (request.len != 8)
	{
		CreateExeption(0x86, 0x03, response);
		return ReqState::RESPONSE;
	}
	else if (CheckChain(start, 1))
	{
		CreateEvents(start, 1, req->data);
		rsp->function = 0x06;
		rsp->startAddress = req->startAddress;
		rsp->data[0] = req->data[0];
		rsp->data[1] = req->data[1];
		response->len = 6;
	}
	else
	{
		CreateExeption(0x86, 0x02, response);
		return ReqState::RESPONSE;
	};
	
	return ReqState::WRITE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ReqState Modbus::ModbusFunction_08(const Message &request, Message *response)
{
	__packed struct Request	{ byte function; word subfunction; __packed union {byte b[249]; word w[124];} data; };
	__packed struct Response { byte function; word subfunction;__packed  union {byte b[249]; word w[124];} data; };
	union { const PDU *reqPDU; const Request *req; };
	union { PDU *rspPDU; Response *rsp; };

	reqPDU = &request.pdu;
	rspPDU = &response->pdu;

	word subfunction = ReverseWord(req->subfunction);

	rsp->function = 0x08;
	rsp->subfunction = req->subfunction;
	response->len = 6;

	switch (subfunction)
	{
		case 00:	// Return Query Data

//			memcpy(rsp->data.b, req->data.b, request.len-2);
			response->len = request.len-2;
			break;
	
		case 01:	// Restart Communications Options

			if (req->data.w[0] == 0 || req->data.w[0] == 0x00FF)
			{
				rsp->data.w[0] = req->data.w[0];

				diagnosticRegister = 0;
				busMessageCount = 0;
				busCommunicationErrorCount = 0;
				busExeptionErrorCount = 0;
				slaveMessageCount = 0;
				slaveNoResponseCount = 0;
				slaveCountNAK = 0;
				slaveBusyCount = 0;
				busCharacterOverrunCount = 0;

				if (listenOnlyMode)
				{
					response->len = 0;
				};
				
				listenOnlyMode = false;
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;
	
		case 02:	// Return Diagnostic Register

			if (req->data.w[0] == 0)
			{
				rsp->data.w[0] = ReverseWord(diagnosticRegister);
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;
	
		case 03:	// Change ASCII Input Delimiter

			if (req->data.b[1] == 0)
			{
				rsp->data.w[0] = req->data.w[0];
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;
	
		case 04:	// Force Listen Only Mode

			if (req->data.w[0] == 0)
			{
				listenOnlyMode = true;
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;
	
		case 10:	// Clear Counters and Diagnostic Register

			if (req->data.w[0] == 0)
			{
				rsp->data.w[0] = 0;

				diagnosticRegister = 0;
				busMessageCount = 0;
				busCommunicationErrorCount = 0;
				busExeptionErrorCount = 0;
				slaveMessageCount = 0;
				slaveNoResponseCount = 0;
				slaveCountNAK = 0;
				slaveBusyCount = 0;
				busCharacterOverrunCount = 0;
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;
	
		case 11:	// Return Bus Message Count

			if (req->data.w[0] == 0)
			{
				rsp->data.w[0] = ReverseWord(busMessageCount);
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;
	
		case 12:	// Return Bus Communication Error Count

			if (req->data.w[0] == 0)
			{
				rsp->data.w[0] = ReverseWord(busCommunicationErrorCount);
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;
	
		case 13:	// Return Bus Exeption Error Count

			if (req->data.w[0] == 0)
			{
				rsp->data.w[0] = ReverseWord(busExeptionErrorCount);
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;
	
		case 14:	// Return Slave Message Count

			if (req->data.w[0] == 0)
			{
				rsp->data.w[0] = ReverseWord(slaveMessageCount);
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;
	
		case 15:	// Return Slave No Response Count

			if (req->data.w[0] == 0)
			{
				rsp->data.w[0] = ReverseWord(slaveNoResponseCount);
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;
	
		case 16:	// Return Slave NAK Count

			if (req->data.w[0] == 0)
			{
				rsp->data.w[0] = ReverseWord(slaveCountNAK);
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;
	
		case 17:	// Return Slave Busy Count

			if (req->data.w[0] == 0)
			{
				rsp->data.w[0] = ReverseWord(slaveBusyCount);
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;
	
		case 18:	// Return Bus Character Overrun Count

			if (req->data.w[0] == 0)
			{
				rsp->data.w[0] = ReverseWord(busCharacterOverrunCount);
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;
	
		case 20:	// Clear Overrun Counter and Flag

			if (req->data.w[0] == 0)
			{
				rsp->data.w[0] = 0;
				busCharacterOverrunCount = 0;
			}
			else
			{
				CreateExeption(0x88, 3, response);
			};

			break;

		default:

			CreateExeption(0x88, 1, response);
	}
	
	return ReqState::RESPONSE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ReqState Modbus::ModbusFunction_16(const Message &request, Message *response)
{
	__packed struct Request	{ byte function; word startAddress; word quantity; byte count; byte data[246]; };
	__packed struct Response	{ byte function; word startAddress; word quantity; };
	union { const PDU *reqPDU; Request *req; };
	union { PDU *rspPDU; Response *rsp; };

	reqPDU = &request.pdu;
	rspPDU = &response->pdu;

	word start = ReverseWord(req->startAddress);
	word quantity = ReverseWord(req->quantity);
	word count = req->count;

	if (quantity < 1 || quantity > 123 || count != quantity*2 || request.len != (9+count))
	{
		CreateExeption(0x90, 0x03, response);
		return ReqState::RESPONSE;
	}
	else if (CheckChain(start, quantity))
	{
		CreateEvents(start, quantity, req->data);
		rsp->function = 16;
		rsp->startAddress = req->startAddress;
		rsp->quantity = req->quantity;
		response->len = 6;
	}
	else
	{
		CreateExeption(0x90, 0x02, response);
		return ReqState::RESPONSE;
	};
	
	return ReqState::WRITE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ReqState Modbus::ModbusFunction(const Message &req, Message *rsp)
{
//	if (_debug) printf("Function %u\n", req.pdu.function);

	switch (req.pdu.function)
	{
		case 0x03:	return ModbusFunction_03(req, rsp);
		case 0x04:	return ModbusFunction_04(req, rsp);
		case 0x06:	return ModbusFunction_06(req, rsp);
		case 0x08:	return ModbusFunction_08(req, rsp);
		case 0x10:	return ModbusFunction_16(req, rsp);

		default:

			CreateExeption(req.pdu.function | 0x80, 1, rsp);
			return ReqState::RESPONSE;
	};
	
//	return ReqState::WAIT;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Modbus::Request()
{
//	int i;
	word	start;
	word	count;
	byte	*data;
//	Var		*var;
	VarEvent *varEvent;

	switch (state.value)
	{
		case ReqState::WAIT:

//			TRACELINE;

			if (reqMessage.ready)
			{
				slaveMessageCount += 1;
				state = ModbusFunction(reqMessage, &rspMessage);
				prevAccessTime = GetMilliseconds();
			};

			break;

		case ReqState::WRITE:
		
			if (numEvents > 0)
			{
				varEvent = varsEvent + numEvents - 1;
				start = varEvent->start;
				count = varEvent->count;
                data = varEvent->data;

				numEvents -= 1;

				if (!varEvent->var->WriteEvent(acsRights, start, count, data))
				{
					state = state.RESPONSE;
					CreateExeption(reqMessage.pdu.function | 0x80, 4, &rspMessage);
//					if (_debug) printf("Write exeption 0x04, Address=0x%04X\n", varEvent->var->Address());
				};
			}
			else
			{
				state = state.WAIT;
				reqMessage.ready = false;
				rspMessage.ready = true;
			};

			break;

		case ReqState::READ:

			if (numEvents > 0)
			{
				varEvent = varsEvent + numEvents - 1;
				start = varEvent->start;
				count = varEvent->count;
                data = varEvent->data;

                numEvents -= 1;

				if (!varEvent->var->ReadEvent(start, count, data))
				{
					state = state.RESPONSE;
					CreateExeption(reqMessage.pdu.function | 0x80, 4, &rspMessage);
//					if (_debug) printf("Read exeption 0x04, Address=0x%04X\n", varEvent->var->Address());
				};
			}
			else
			{
				state = state.WAIT;
				rspMessage.ready = true;
				reqMessage.ready = false;
			};

			break;

		case ReqState::RESPONSE:

			state = state.WAIT;
			reqMessage.ready = false;
			rspMessage.ready = true;
			break;

	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Modbus::Update()
{
//	static word p = 0;

//	word t;
//	dword asd;

//	DataCRC CRC;

	switch (mode)
	{
		case 0:

			if (rspMessage.ready)
			{
				if (reqMessage.address == 0 || rspMessage.len == 0 || listenOnlyMode)
				{
					rspMessage.ready = false;
					slaveNoResponseCount += 1;
				}
				else
				{
					rspMessage.address = reqMessage.address;

					byte *s = &rspMessage.address + rspMessage.len;
				
					*(word*)s = GetCRC16(&rspMessage.address, rspMessage.len); 
					rspMessage.len += 2;
					rspMessage.ready = true;

					if (rspMessage.pdu.function & 0x80)
					{
						busExeptionErrorCount += 1;
					};

					writeBuffer.len = rspMessage.len;
					writeBuffer.data = &rspMessage.address;

					if (comPort.Write(&writeBuffer))
					{
						mode = 2;
					}
					else
					{
						rspMessage.ready = false;
					};
				};
			}
			else if (!reqMessage.ready)
			{
				// Чтение	

				readBuffer.len = readBuffer.maxLen = sizeof(reqMessage.pdu) + sizeof(reqMessage.address);
				readBuffer.data = &reqMessage.address;

				if (comPort.Read(&readBuffer, -1, time35))
				{
					mode = 1;
				}
				else
				{
					reqMessage.ready = false;
				};
			};

			break;

		case 1:

			if (!comPort.Update())
			{
				reqMessage.ready = readBuffer.recieved && (reqMessage.address == netAddress || reqMessage.address == 0);
				reqMessage.len = readBuffer.len;

				if (reqMessage.ready)
				{
					if (GetCRC16(&reqMessage.address, reqMessage.len) != 0)
					{
						busCommunicationErrorCount += 1;
						reqMessage.ready = false;
					};
				};

				mode = 0;

				break;
			};

			break;

		case 2:

			if (!comPort.Update())
			{
				mode = 0;
				rspMessage.ready = false;
			};

			break;
	};

	Request();

	accessTimeout = GetMilliseconds() - prevAccessTime;

	if (accessTimeout > 0x7FFFFFFF)
	{
		prevAccessTime += accessTimeout - 0x7FFFFFFF;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef WIN32

bool Modbus::ConnectTCP(const char *adr, word port)
{
	accptSocket = SOCKET_ERROR;

	WSADATA wsaData;
	
	if (WSAStartup( MAKEWORD(2,2), &wsaData) != NO_ERROR)
	{
		cputs("Error at WSAStartup()\n");
		return false;
	}
	else
	{
		cputs("WSAStartup() ... OK\n");
	};

	lstnSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	cputs("Creating socket ... ");
	
	if (lstnSocket == INVALID_SOCKET)
	{
		cputs("ERROR!!!\n");
	    WSACleanup();
		return false;
	}
	else
	{
		cputs("OK\n");
	};

	int iMode = 1;

	if (ioctlsocket(lstnSocket, FIONBIO, (u_long FAR*) &iMode) != 0)
	{
		cputs("Set socket to nonblocking mode FAILED!!!\n");
	};

	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);

	if(setsockopt(lstnSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&bOptVal, bOptLen) != SOCKET_ERROR)
	{
		cputs("TCP_NODELAY enabled\n");
	};

	sockaddr_in srvc;

	srvc.sin_family = AF_INET;
	srvc.sin_addr.s_addr = inet_addr(adr);
	srvc.sin_port = ReverseWord(port);

	if (bind(lstnSocket, (SOCKADDR*)&srvc, sizeof(srvc)) == SOCKET_ERROR )
	{
		cputs("bind() socket failed\n" );
		closesocket(lstnSocket);
	    WSACleanup();
		return false;
	}

	if (listen(lstnSocket, 1 ) == SOCKET_ERROR )
	{
		cputs("Error listening on socket.\n");
		closesocket(lstnSocket);
	    WSACleanup();
		return false;
	};

	return connected = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Modbus::UpdateTCP()
{
	word len;

	if (accptSocket == SOCKET_ERROR)
	{
		if ((accptSocket = accept(lstnSocket, NULL, NULL )) != SOCKET_ERROR)
		{
			cputs("TCP connection established\n");
		};
	}
	else
	{
		if (rspMessage.ready)
		{
			rspMessage.trnsId = reqMessage.trnsId;
			rspMessage.prtclID = reqMessage.prtclID;
			rspMessage.address = reqMessage.address;

			rspMessage.len = ReverseWord(len = rspMessage.len);

			//byte *s = &rspMessage.trnsId + rspMessage.len + 6;
		
			//*(word*)s = GetCRC16(&rspMessage.address, rspMessage.len); 
			//rspMessage.len += 2;


			send(accptSocket, (char*)&rspMessage.trnsId, len + 6, 0); 

			rspMessage.ready = false;

		}
		else if (!reqMessage.ready)
		{
			// Чтение	

			int len = recv(accptSocket, (char*)&reqMessage.trnsId, sizeof(reqMessage.pdu) + 7, 0); 

			if (len > 7)
			{
				reqMessage.len = ReverseWord(reqMessage.len);
				len -= 6;

				reqMessage.ready = (len == reqMessage.len);
				reqMessage.len += 2;
			}
			else if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				closesocket(accptSocket);
				cputs("TCP connection closed\n");
				accptSocket = SOCKET_ERROR;
			};
		};

		Request();
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif
