
#include <iostream>

using namespace std;

#include <netlink/socket.h>



int main() {

	NL::init();

	cout << "\nStarting server...";
	cout.flush();

	try {

		NL::Socket server(5000);

		NL::Socket* clientConnection = server.accept();

		char buffer[256];
		buffer[255] = '\0';

		while(clientConnection->read(buffer,255)) {
			cout << "\nMessage: " << buffer;
			cout.flush();
		}

		delete clientConnection;

		cout << "\nClient disconnected. Exit...";

	}

	catch(NL::Exception e) {

		cout << "\n***Error*** " << e.what();
	}


	return 0;
}
