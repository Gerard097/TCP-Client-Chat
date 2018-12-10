#include <iostream>
#include "ClientTCP.h"

int main(int argc, char **argv) {

	ClientTCP client;

	if (client.Init())
		client.Start();
	else
		return 1;

	return 0;
}