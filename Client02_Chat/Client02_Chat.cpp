// Client02_Chat.cpp : Defines the entry point for the console application.
//

#include "../../BaselineNetworkCode/Thread.h"
#include "../../BaselineNetworkCode/BasePacket.h"

#include "../../BaselineNetworkCode/PacketFactory.h"

#include <iostream>
using namespace std;
#include <assert.h>
#include <conio.h>
#define _CRT_SECURE_NO_DEPRECATE
//#include <windows.h>
//#include <winsock2.h>

#pragma warning( disable: 4996 )


enum Colors
{
   ColorsText = 15,
   ColorsUsername = 4,
   ColorsResponseText = 6,
   ColorsNormal = 2
};
void  SetConsoleColor( int color )
{
   // change the text color
   HANDLE hConsole;
   hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   SetConsoleTextAttribute( hConsole, color );
}

class Fruitadens : public Threading::CChainedThread <BasePacket*>
{
public:
   Fruitadens() : CChainedThread( true, 30 ),
                  m_clientSocket( 0 ),
                  m_isConnected( false ),
                  m_port( 0 )
   {
      memset( &m_ipAddress, 0, sizeof( m_ipAddress ) );
   }

   bool  Connect( const char* serverName, int port )// work off of DNS
   {
      if( SetupConnection( serverName, port ) == false )
      {
         assert( 0 );
         return false;
      }
      return true;
   }
   bool  Disconnect()
   {
      Cleanup();
      closesocket( m_clientSocket );
   }
protected:

   bool  SetupConnection( const char* serverName, int port )
   {
      Pause();
      m_clientSocket = socket(AF_INET, SOCK_STREAM, 0);
      if (m_clientSocket == SOCKET_ERROR)
      {
         cout << "Client: The Socket is stuck sir! It wont open!\n";
         return false;
      }
      struct hostent *host_entry;
      if ((host_entry = gethostbyname( serverName )) == NULL)
      {
         cout << "Client: I'm lost =.=; Wheres the host?\n";
      }
      struct sockaddr_in     server;
      server.sin_family = AF_INET;
      server.sin_port = htons(port);
      server.sin_addr.s_addr = *(unsigned long*) host_entry->h_addr;

      if (connect( m_clientSocket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
      {
         cout << "Client: ****ing server wont let me connect dude.\n";
      }
      cout << "Client: I-AM-RUNNING!\n";

      if( SetSocketToNonblock( m_clientSocket ) == false )
	   {
		   cout << "ERROR: cannot set socket to non-blocking" << endl;
		   //EndClient( m_clientSocket, receiveThread );
         Cleanup();
         closesocket( m_clientSocket );
         m_clientSocket = 0;
         return false;
	   }

      m_isConnected = true;
      Resume();

      return true;
   }

   int  ProcessInputFunction()
   {
      return 1;
   }

   void  SerializePacketOut( const BasePacket* packet )
   {
      const int bufferSize = 2048;
      U8 buffer[2048];

      int offset = 0;
      packet->SerializeOut( buffer, offset );
      SendPacket( buffer, offset );
   }
   bool  SendPacket( const U8* buffer, int length )
   {
      int nBytes = send( m_clientSocket, reinterpret_cast<const char*>( buffer ), length, 0 );
      if( nBytes == SOCKET_ERROR || nBytes < length )
      {
         cout << "Client: It wont go through sir!!\n";
         return false;
      }

      return true;
   }

   int            m_clientSocket;
   bool           m_isConnected;
   sockaddr_in    m_ipAddress;
   U16            m_port;
};

//-----------------------------------------------------------------------------

class FruitadensChat : public Fruitadens
{
public:
   FruitadensChat() : Fruitadens() {}
   bool  Login( const string& username, const string& password )
   {
      PacketLogin login;
      login.loginKey = "deadbeef";
      login.uuid = username;
      login.username = username;
      login.loginKey = password;

      SerializePacketOut( &login );

      return true;
   }
   bool  Logout( const string& username, const string& password )
   {
      PacketLogout logout;
      SerializePacketOut( &logout );

      return true;
   }
   bool	SendMessage( const string& message )
   {
      PacketChatToServer chat;
      chat.gameTurn = 3;

      chat.message = message;
      SerializePacketOut( &chat );

      return true;
   }

   bool  ChangeChannel( string& channel )
   {
      PacketChangeChatChannel channelChange;
      channelChange.chatChannel = channel;
      SerializePacketOut( &channelChange );
      return true;
   }

protected:
   int  ProcessOutputFunction()
   {
      const int bufferLength = 2048;
	   U8 buffer[ bufferLength ];

      int numBytes = recv( m_clientSocket, (char*) buffer, bufferLength, NULL );
		if( numBytes != SOCKET_ERROR)
		{
			buffer[ numBytes ] = 0;// NULL terminate
			cout << "RECEIVED: " << buffer << endl;
         ChatPacketFactory factory;
         int offset = 0;
         while( offset < numBytes )
         {
            BasePacket* packetIn;
            if( factory.Parse( buffer, offset, numBytes, &packetIn ) == true )
            {
               switch( packetIn->packetType )
               {
               case PacketType_Chat:
                  {
                     switch( packetIn->packetSubType )
                     {
                    /* case PacketChatToServer::ChatType_ChatToServer:
                        {
                           PacketChatToServer* chat = static_cast<PacketChatToServer*>( packetIn );
                        }
                        return true;*/
                     case PacketChatToServer::ChatType_ChatToClient:
                        {
                           PacketChatToClient* chat = static_cast<PacketChatToClient*>( packetIn );

                           SetConsoleColor( ColorsNormal );
                           cout << "Message sent by "; 
                           SetConsoleColor( ColorsUsername );
                           cout << chat->username << endl;
                           SetConsoleColor( ColorsNormal );
                           cout << "Message "; 
                           SetConsoleColor( ColorsResponseText );
                           cout << chat->message << endl;
                           SetConsoleColor( ColorsNormal );
                        }
                        break;

                     case PacketChatToServer::ChatType_ChangeChatChannel:
                        {
                           PacketChangeChatChannel* channel = static_cast<PacketChangeChatChannel*>( packetIn );

                           SetConsoleColor( ColorsNormal );
                           cout << "Channel change "; 
                         /*  SetConsoleColor( ColorsUsername );
                           cout << channel-> << endl;*/
                           SetConsoleColor( ColorsNormal );
                           cout << "Channel "; 
                           SetConsoleColor( ColorsText );
                           cout << channel->chatChannel << endl;
                           SetConsoleColor( ColorsNormal );
                        }
                        break;
                     }
                  }
               }
               delete packetIn;
            }
         }
		}
      
      return 1;
   }
};
//--------------------------------------------------------------------

bool BreakBufferUp( const char* buffer, string& newUserLogin, string& message, string& channel );
void	RequestUserLoginCredentials( string& username, string& password );
void	SendLoginCedentialsToServer( FruitadensChat& fruit, const string& username, const string& password );

//--------------------------------------------------------------------

int main()
{
   cout << endl << endl
         << "Client communications app." << endl;

   

   InitializeSockets();

   FruitadensChat fruity;
   fruity.Connect( "localhost", 9600 );

   string username, password;
	RequestUserLoginCredentials( username, password );
	if( username.size() == 0 )
	{
      fruity.Cleanup();
      ShutdownSockets();
      return 1;
	}

   SendLoginCedentialsToServer( fruity, username, password );

   char buffer[256];

   while( 1 )
   {
      //cin >> buffer;
      
      cout << "Type your next message. Use /channel_name to select the channel. " << endl << ">";
      scanf( "%s", buffer );
      if (strcmp( buffer, "exit" ) == 0)
      {
         break;
      }

      string newUserLogin, message, channel;

      BreakBufferUp( buffer, newUserLogin, message, channel );

      if( newUserLogin.size() )
      {
      }
      else if( message.size() )
      {
         if( fruity.SendMessage( message ) == false )
         {
            cout << "Problem with sending the message" << endl;
	         break;
         }
      }
      else if( channel.size() )
      {
         if( fruity.ChangeChannel( channel ) == false )
         {
            cout << "Problem changing channel" << endl;
	         break;
         }
      }

   }

   ShutdownSockets();

   getch();
	return 0;
}

//--------------------------------------------------------------------------

void	FindCommands( string& originalText, string& command )
{
	//channel.clear();
	int length = originalText.size();
	int position = originalText.find_first_of( '/' );
	if( position == string::npos )
	{
		return;
	}
	while( position < length && originalText[ position ] == '/' )// allowing users to enter too many slashes by accident
	{
		position++;
	}

	if( position >= static_cast<int>( originalText.size() ) ) 
	{
		return;
	}

	int nextPosition = originalText.find_first_of( ' ', position );// go until we find a space. treat that as the channel marker.
	char buffer[MAX_PATH];
	char newText[MAX_PATH];// todo, needs to be much longer
	newText[0] = 0;
	if( nextPosition == string::npos ) 
	{
		int copyLength = length - position;
		originalText.copy( buffer, copyLength, position );
		buffer[ copyLength ] = 0;
	} 
	else 
	{
		int copyLength = nextPosition - position;
		originalText.copy( buffer, nextPosition - position, position );
		buffer[ copyLength ] = 0;

		copyLength = nextPosition - position;
		originalText.copy( newText, length - nextPosition, nextPosition +1 );
		newText[ copyLength ] = 0;
	}
	originalText = newText;
	command = buffer;
}

//--------------------------------------------------------------------------

bool BreakBufferUp( const char* buffer, string& newUserLogin, string& message, string& channel )
{
   string textBuffer = buffer;
   FindCommands( textBuffer , channel );
   message = textBuffer;
   return true;
}


//--------------------------------------------------------------------------

void	RequestUserLoginCredentials( string& username, string& password )
{
	do{// todo, request password
		char buffer[256];
      SetConsoleColor( ColorsNormal );
		cout<< "Enter username: ";
      SetConsoleColor( ColorsUsername );
		cin >> buffer;
      SetConsoleColor( ColorsResponseText );
		if( strlen( buffer ) < 1 ){// todo, needs more validation
			cout << "username " << buffer << " is invalid" << endl;
		} else {
			username = buffer;
		}
	} while ( username.size() < 1 );
	password = "password";
}

//--------------------------------------------------------------------------

void	SendLoginCedentialsToServer( FruitadensChat& fruity, const string& username, const string& password )
{
   if( fruity.Login( username, password ) == false )
   {
      cout << "Client: Login went badly!" << endl;
   }
}

//--------------------------------------------------------------------------
