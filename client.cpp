#include "ClientSocket.h"

int main()
{
	ClientSocket client("127.0.0.1", 5555);
	client.start();
}