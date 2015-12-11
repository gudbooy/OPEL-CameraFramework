#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>    //dbus 헤더파일 
static void send_init(DBusConnection *connection)  
{
	DBusMessage *message;
	message = dbus_message_new_signal ("/org/opel/camera/daemon", "org.opel.camera.daemon", "recInit");   // On이라는 signal을 해당 주소에 emit해라

	/* Send the signal */
	dbus_connection_send (connection, message, NULL);    //이 부분에서 데이터를 전송함
	dbus_message_unref (message);
}
static void send_start(DBusConnection *connection)
{
	DBusMessage *message;
	message = dbus_message_new_signal ("/org/opel/camera/daemon", "org.opel.camera.daemon", "recStart");   // On이라는 signal을 해당 주소에 emit해라

	/* Send the signal */
	dbus_connection_send (connection, message, NULL);    //이 부분에서 데이터를 전송함
	dbus_message_unref (message);
}
static void send_stop(DBusConnection *connection)
{
	DBusMessage *message;
	message = dbus_message_new_signal ("/org/opel/camera/daemon", "org.opel.camera.daemon", "recStop");   // On이라는 signal을 해당 주소에 emit해라

	/* Send the signal */
	dbus_connection_send (connection, message, NULL);    //이 부분에서 데이터를 전송함
	dbus_message_unref (message);
}
static void send_close(DBusConnection *connection)
{
	DBusMessage *message;
	message = dbus_message_new_signal ("/org/opel/camera/daemon", "org.opel.camera.daemon", "OpenCVClose");   // On이라는 signal을 해당 주소에 emit해라

	/* Send the signal */
	dbus_connection_send (connection, message, NULL);    //이 부분에서 데이터를 전송함
	dbus_message_unref (message);
}
static void send_off(DBusConnection *connection)  // send_off에서는 integer형 data를 send하는 방법  
{
	DBusMessage *message; 
	DBusMessageIter args, args2;
	DBusError err;
	dbus_int32_t q = 1;    //보낼 데이터 1 번
	dbus_int32_t hi = 2;   //보낼 데이터 2번
	int d;
	dbus_error_init(&err);
	message = dbus_message_new_signal ("/org/share/linux", "org.share.linux", "off");  //signal을 off로 보내짐
	dbus_message_iter_init_append(message, &args);   // 데이터를 포장하는 부분
	if( !dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &q))  // q=1 append시켜 message에 추가한다.
		exit(1);
	if( !dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &hi)) // hi=2 라는 값을 append 시켜 message에 추가한다.
		exit(1);

	/* Send the signal */
	dbus_connection_send (connection, message, NULL);  //send를 하게되면 데이터를 보냄
	dbus_message_unref (message);
}
int main (int argc, char **argv)
{

	DBusConnection *connection;
	DBusError error;
	dbus_error_init (&error);
	connection = dbus_bus_get (DBUS_BUS_SYSTEM, &error);
	if (!connection)
	{
		printf ("Failed to connect to the D-BUS daemon: %s", error.message);
		dbus_error_free (&error);
		return 1;
	}
	if ( argc == 1 )
	{
		return 0;
	}
	int i;
	for ( i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i],"-i") )
		{
			send_init(connection);
		}
		else if(!strcmp(argv[i], "-s") )
		{
			send_start(connection);
		}
		else if(!strcmp(argv[i], "-t"))
		{
			send_stop(connection);
		}
		else if(!strcmp(argv[i], "-c"))
		{
			send_close(connection);
		}
		else if ( !strcmp(argv[i],"-q") )
		{
			send_off(connection);
		}
	}
	return 0;
}
