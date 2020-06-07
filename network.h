// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2006 *nixCoders team - don't forget to credit us

#pragma once

// general
typedef int socket_t;

typedef struct {
	char	hostname[64];
	int		port;
} server_t;

// ET/q3 related
#define NET_PROTOCOL				"84"
#define NET_Q3MARKER				"\xff\xff\xff\xff"
#define NET_STATUS_REQUEST			(NET_Q3MARKER "getstatus")
#define NET_STATUS_RESPONSE			(NET_Q3MARKER "statusResponse")
#define NET_STATUS_TIMEOUT			300
#define NET_SERVERS_REQUEST			(NET_Q3MARKER "getservers " NET_PROTOCOL)
#define NET_SERVERS_RESPONSE		(NET_Q3MARKER "getserversResponse")
#define NET_SERVERS_RESPONSE_END	"\\EOT"
#define NET_SERVERS_TIMEOUT			3000
#define NET_ET_MASTER_SERVER		(server_t){ "etmaster.idsoftware.com", 27950 }

// irc related
#define IRC_NOTICE			"NOTICE"
#define IRC_JOIN			"JOIN"
#define IRC_NICK			"NICK"
#define IRC_PART			"PART"
#define IRC_QUIT			"QUIT"
#define IRC_PING			"PING "
#define IRC_PONG			"PONG "
#define IRC_PRIVMSG			"PRIVMSG"
#define IRC_REGISTRED		"001"
#define IRC_LIST_NAMES		"353"
#define IRC_TOPIC			"332"
#define IRC_TOPIC_CHANGE	"TOPIC"
#define IRC_ACTION			"\1ACTION"
#define IRC_CTCP_VERSION	"\1VERSION\1"
#define IRC_KICK			"KICK"
#define IRC_MODE			"MODE"
#define IRC_TOPIC_SIZE		390
#define IRC_NICK_SIZE		30
#define IRC_BOT_PREFIX		"nex_"
#define IRC_BOT_CHANNEL		"#nexus-online"
#define IRC_002				"002"

#define IRC_CHAT_KEY			K_F10
#define IRC_CHAT_TIME			150
// make defines 'dynamic' later again
#define IRC_CHAT_W				620
#define IRC_CHAT_X				10
#define IRC_CHAT_H				200
#define IRC_CHAT_Y				270
#define IRC_CHAT_TEXT_SIZE		0.16f
#define IRC_CHAT_SPACE			2
#define IRC_CHAT_BORDER_SIZE	2

class CNexNetwork {
	public:
		void init();
		socket_t serverConnect(server_t *server, int type);
		bool sendRaw(socket_t socket, char *buffer, int size);
		bool sendText(socket_t socket, char *format, ...);
		bool sendIrcChannel(char *format, ...);
		void registerIrcCommands();
		void addChannelMsg(char *msg);
		void sendIrcStatus();
		void saveIrcNickToConfig();
		void getSpectators();
	private:
};

extern CNexNetwork nexNetwork;

// network threads
DWORD hIRC(LPVOID);	// irc
DWORD hSPC(LPVOID);	// spectator listing
DWORD hVER(LPVOID);	// are we up2date
DWORD hDET(LPVOID);	// detection checker

// irc related console cmds
void sendCommand (void);
void nickCommand (void);

// save channel traffic
#define IRC_MAX_MESSAGES 50
typedef struct {
	char *msg;
//	bool show;
} ircMessage_t;

extern ircMessage_t ircMessages[IRC_MAX_MESSAGES];

