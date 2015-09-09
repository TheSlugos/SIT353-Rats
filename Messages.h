enum MESSAGEID { JOIN, JOINACCEPT, REJECTED, UPDATE, MAPDATA, MAPREQUEST, COMMAND };
enum REJECTREASON { GAMEFULL, ALREADYJOINED };

class MsgJoin
{
public:
	int _Id;
	
	MsgJoin() : _Id(JOIN) {}
};

class MsgAccepted
{
public:
	int _Id;
	int _PlayerNo;

	MsgAccepted(int playerNo) : _Id(JOINACCEPT), _PlayerNo(playerNo) {}
};

class MsgRejected
{
public:
	int _Id;
	int _Reason;

	MsgRejected(int reason) : _Id(REJECTED), _Reason(reason) {}
};

class MsgMapRequest
{
public:
	int _Id;

	MsgMapRequest() : _Id(MAPREQUEST) {}
};

class MsgPlayerCommand
{
public:
	int _Id;
	int _PlayerId;
	char _Command;

	MsgPlayerCommand(int playerId, char command) : _Id(COMMAND), _PlayerId(playerId), _Command(command) {}
};

//class MsgAddLink
//{
//public:
//	int _Id;
//	int _StartX;
//	int _StartY;
//	int _DestX;
//	int _DestY;
//
//	MsgAddLink(int sx, int sy, int dx, int dy) : _Id(ADDLINK), _StartX(sx), _StartY(sy),
//		_DestX(dx), _DestY(dy) {}
//};

//class MsgRemoveLinks
//{
//public:
//	int _Id;
//	int _X;
//	int _Y;
//
//	MsgRemoveLinks(int x, int y) : _Id(REMOVELINKS), _X(x), _Y(y) {}
//};
