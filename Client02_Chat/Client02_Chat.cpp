// Client02_Chat.cpp : Defines the entry point for the console application.
//

#include "../../BaselineNetworkCode/Thread.h"
#include "../../BaselineNetworkCode/BasePacket.h"

#include "../../BaselineNetworkCode/PacketFactory.h"
#include "../../BaselineNetworkCode/Fruitadens.h"
#include "../../BaselineNetworkCode/Pyroraptor.h"

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
         SetConsoleColor( ColorsText );
         Log( "Data has come in" );
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
                  case PacketType_Login:
                  {
                     switch( packetIn->packetSubType )
                     {
                     case PacketLogin::LoginType_Login:
                        {
                           PacketLogin* login = static_cast<PacketLogin*>( packetIn );
                           SetConsoleColor( ColorsNormal );
                           cout << "User login "; 
                           SetConsoleColor( ColorsUsername );
                           cout << login->username << endl;
                           SetConsoleColor( ColorsNormal );
                        }
                        break;
                     case PacketLogin::LoginType_PacketLogoutToClient:
                        {
                           PacketLogoutToClient* login = static_cast<PacketLogoutToClient*>( packetIn );
                           SetConsoleColor( ColorsNormal );
                           cout << "User logged out "; 
                           SetConsoleColor( ColorsUsername );
                           cout << login->username << endl;
                           SetConsoleColor( ColorsNormal );
                        }
                        break;
                        }
                     }
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

                           SetConsoleColor( ColorsUsername );
                           cout << chat->username ;

                           SetConsoleColor( ColorsNormal );
                           cout << " says "; 

                           SetConsoleColor( ColorsResponseText );
                           cout << chat->message;

                           SetConsoleColor( ColorsNormal );
                           cout << " on chat channel:";

                           SetConsoleColor( ColorsResponseText );
                           cout << chat->chatChannel << endl;
                           SetConsoleColor( ColorsNormal );
                        }
                        break;

                     case PacketChatToServer::ChatType_ChangeChatChannelToClient:
                        {
                           PacketChangeChatChannelToClient* channel = static_cast<PacketChangeChatChannelToClient*>( packetIn );

                           SetConsoleColor( ColorsNormal );
                           cout << "Channel change "; 
                           SetConsoleColor( ColorsUsername );
                           cout << channel->username;
                           SetConsoleColor( ColorsNormal );
                           cout << " changed to channel "; 
                           SetConsoleColor( ColorsText );
                           cout << channel->chatChannel << endl;
                           SetConsoleColor( ColorsNormal );
                        }
                        break;
                     case PacketChatToServer::ChatType_SendListOfChannelsToClient:
                        {
                           PacketChatChannelListToClient* channelList = static_cast<PacketChatChannelListToClient*>( packetIn );

                           SetConsoleColor( ColorsNormal );
                           int num = channelList->chatChannel.size();
                           cout << "Channels for this user are [" << num << "] = {"; 

                           SetConsoleColor( ColorsUsername );
                           for( int i=0; i<num; i++ )
                           {
                              cout << channelList->chatChannel[i];
                              if( i < num-1 )
                                  cout << ", ";
                           }
                           SetConsoleColor( ColorsNormal );
                           cout<< "}" << endl;
                        }
                        break;
                     }
                  }
               }
               delete packetIn;
            }
            else 
            {
               offset = numBytes;
            }
         }
		}
      
      return 1;
   }
};
//--------------------------------------------------------------------

bool BreakBufferUp( const string& buffer, string& newUserLogin, string& message, string& channel );
void	RequestUserLoginCredentials( string& username, string& password );
void	SendLoginCedentialsToServer( FruitadensChat& fruit, const string& username, const string& password );

//--------------------------------------------------------------------

int main()
{
   cout << endl << endl
         << "Client communications app." << endl;

   

   InitializeSockets();

   string serverName = "localhost";
   serverName = "chat.mickey.playdekgames.com";
   FruitadensChat fruity;
   fruity.Connect( serverName.c_str(), 9600 );

   Pyroraptor pyro;
   fruity.AddOutputChain( &pyro );

   string username, password;
   RequestUserLoginCredentials( username, password );
	if( username.size() == 0 )
	{
      fruity.Cleanup();
      ShutdownSockets();
      return 1;
	}

   SendLoginCedentialsToServer( fruity, username, password );

   //char buffer[256];

   while( 1 )
   {
      //cin >> buffer;
      
      cout << "Type your next message. Use /channel_name to select the channel. " << endl << ">";
      //scanf( "%s", buffer );
      std::string buffer;
      std::getline(std::cin, buffer);

      if (strcmp( buffer.c_str(), "exit" ) == 0)
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

   SetConsoleColor( ColorsText );
   cout << "press any key to exit" << endl;
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

bool BreakBufferUp( const string& buffer, string& newUserLogin, string& message, string& channel )
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
