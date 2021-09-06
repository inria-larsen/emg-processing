#include "EmgTcp.h"

#define CMD_PORT 50040
#define IM_EMG_PORT 50043

	EmgTcp::EmgTcp(){
		cmdPort_ = CMD_PORT;
		imEmgPort_ = IM_EMG_PORT;		
	}

	EmgTcp::EmgTcp(std::string servIpAdd)
	:
	servIpAdd_(servIpAdd),
	cmdPort_(CMD_PORT),
	imEmgPort_(IM_EMG_PORT)
	{

	}

	bool EmgTcp::isCmdConnected(void){
		return cmdConnected_;
	}
	bool EmgTcp::isImEmgConnected(void){
		return imEmgConnected_;
	}

	bool EmgTcp::isStreaming(void){
		return streamingData_;
	}

	void EmgTcp::setIpAdd(const std::string servIpAdd){
		servIpAdd_ = servIpAdd;
	}

	bool EmgTcp::connect2Server(){
		
		//Initialize addr_ struct
		const char* cServIpAdd = const_cast<char*>(servIpAdd_.c_str());
		memset (&addr_, 0, sizeof(addr_));

		addr_.sin_family = AF_INET;

		if (resolveHostName(cServIpAdd, &(addr_.sin_addr)) != 0 ) {
        	inet_pton(PF_INET, cServIpAdd, &(addr_.sin_addr));        
    	}

		//Connect all ports (command, emg data, im_emg, etc...)
		bool ok = connectCmdTcp();
			if(!ok)	return ok;
		ok &= connectImEmgTcp();

		return ok;
	}

	bool EmgTcp::connectCmdTcp(){

		//assign correct port
		addr_.sin_port = htons(cmdPort_);

    	//create a socket
		cmdSock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	 	if (cmdSock_ < 0) {
			yError("Problem connecting to server [socket]");
			//throw std::runtime_error("Problem connecting to server [socket]");
			cmdConnected_ = false;
			return cmdConnected_;
	   	}

	   	//try to connect
   	    if (connect(cmdSock_, (struct sockaddr*)&addr_, sizeof(addr_)) != 0)
   	    {
	        closeCmdSock();
			// throw std::runtime_error("Problem connecting to server");
			yError("Problem connecting to cmd port");
			cmdConnected_ = false;
			return cmdConnected_;
    	}

    	std::cout << std::endl << "Connected to: "<< readCmdReply();
    	std::cout << std::endl;
    	cmdConnected_ = true;

    	return cmdConnected_;

    	
	}


	void EmgTcp::closeCmdSock(void){

		if(close(cmdSock_) != 0){
			//throw std::runtime_error("Failed to close command socket");
			yFatal("Failed to close command socket");
		}

		cmdConnected_ = false;
	}

	bool EmgTcp::connectImEmgTcp(){

		//assign correct port
		addr_.sin_port = htons(imEmgPort_);

    	//create a socket
		imEmgSock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	 	if (imEmgSock_ < 0) {
			//throw std::runtime_error("Problem connecting to server [socket]");
			yError("Problem connecting to Emg Port [socket]");
			imEmgConnected_ = false;
			return imEmgConnected_;
	   	}

	   	//try to connect
   	    if (connect(imEmgSock_, (struct sockaddr*)&addr_, sizeof(addr_)) != 0)
   	    {
	        closeImEmgSock();
			//throw std::runtime_error("Problem connecting to server");
			yError("Problem connecting to Emg Port");
			imEmgConnected_ = false;
			return imEmgConnected_;
    	}

    	imEmgConnected_ = true;
		yInfo("Connected to Emg Port");
    	return imEmgConnected_;
	}

	void EmgTcp::closeImEmgSock(void){

		if(close(imEmgSock_) != 0){
			//throw std::runtime_error("Failed to close IM EMG socket");
			yError("Failed to close IM EMG socket");
		}

		imEmgConnected_ = false;
	}



	int EmgTcp::resolveHostName(const char* hostname, struct in_addr* addr) 
	{
	    struct addrinfo *res;
	  
	    int result = getaddrinfo (hostname, NULL, NULL, &res);
	    if (result == 0)
	    {
	        memcpy(addr, &((struct sockaddr_in *) res->ai_addr)->sin_addr, sizeof(struct in_addr));
	        freeaddrinfo(res);
    	}

    	return result;
	}

	int EmgTcp::writeCmd(std::string cmd){

		ssize_t n = send(cmdSock_, cmd.c_str(), strlen(cmd.c_str()), 0); 
		if (n < 0)	
			//throw std::runtime_error("Problem sending command to server");
			yError("Problem sending command to server!");

		return (int) n;

	}

	std::string  EmgTcp::readCmdReply(){

        int bytesAvail = 0;
        //wait for reply
        while (bytesAvail == 0 && ioctl(cmdSock_, FIONREAD, &bytesAvail) >= 0)
        {
                usleep(100 * 1000); /* 100 ms */
                printf(".");
        }
        if (bytesAvail > 0)
        {
                char *tmp = (char*)malloc(bytesAvail);
                recv(cmdSock_, tmp, bytesAvail, 0);
                tmp[bytesAvail] = '\0';

                std::string cmdReply (tmp);

                free(tmp);
                return cmdReply;
        }

        // auto cmdReply = std::make_shared<std::string>("");//returns empty string
        return std::string("");
	}
	
	std::string  EmgTcp::sendCmd(std::string cmd){

		if(!isCmdConnected()){
			//throw std::runtime_error("Not Connected to Server!");
			yFatal("Trying to send command but it is NOT connected to Server!");
		}

		writeCmd(cmd);

		return readCmdReply();

	}

	void EmgTcp::configServer(){

		//send configuration commands(please refer to SDK for more information)
		std::cout <<std::endl<< sendCmd("TRIGGER START OFF\r\n\r\n");
		// getchar();
		std::cout <<std::endl<< sendCmd("TRIGGER STOP OFF\r\n\r\n");
		// getchar();
		std::cout <<std::endl<< sendCmd("UPSAMPLE OFF\r\n\r\n");
		// getchar();
		std::cout <<std::endl<< sendCmd("ENDIAN LITTLE\r\n\r\n");
		// getchar();
		std::cout <<std::endl<< sendCmd("UPSAMPLING?\r\n\r\n");
		// getchar();

	}

	void EmgTcp::startDataStream(){

		if(!isImEmgConnected()){
			//throw std::runtime_error("Not Connected to Server data port");
			yFatal("Trying to receive data but it is NOT connected to data port!");
		}
		
		//Check if it is streaming
		if(isStreaming()){
			std::cout << std::endl << "Stream has already started";
			return;
		}


		std::cout <<std::endl<< sendCmd("START\r\n\r\n");
		 //getchar();
		
		streamingData_= true;
	}

	void EmgTcp::stopDataStream(){
		
		if(!isImEmgConnected()){
			//throw std::runtime_error("Not Connected to Server data port");
			yError("Not Connected to Server data port");
		}

		//Check if it is streaming
		if(!isStreaming()){
			std::cout << std::endl << "Stream has already stopped";
			return;
		}
		

		// std::cout <<std::endl<< sendCmd("STOP\r\n\r\n");
		// getchar();
		std::cout <<std::endl<< sendCmd("QUIT\r\n\r\n");

		//close sockets now
		closeCmdSock();
		closeImEmgSock();

		streamingData_= false;
	}

	EmgData EmgTcp::getData(){

		EmgData emgData;
		char dataBuf[SZ_DATA_IM_EMG];
		int count = 0;
		while(1){
			count++;
			if (recv(imEmgSock_, dataBuf, SZ_DATA_IM_EMG, MSG_PEEK) >= SZ_DATA_IM_EMG){
				//actually get the data
				auto nbytes = recv(imEmgSock_, dataBuf, SZ_DATA_IM_EMG, 0);

					float imemgDataFlt[SZ_DATA_IM_EMG / sizeof(float)];
                    memcpy(imemgDataFlt, dataBuf, SZ_DATA_IM_EMG);
                    std::vector<float> aux(std::begin(imemgDataFlt), std::end(imemgDataFlt));
                    emgData.data = aux;

				// std::cout << "\n gotdata: "<<nbytes <<" ";
				// std::cout << "waited for timesteps: \n" << count;
				count = 0;

				// std::cout <<emgData.data[0]<<"\n";
				// std::cout <<emgData.data[1]<<"\n";
				// std::cout <<emgData.data[2]<<"\n";

				// std::cout <<emgData.data[3]<<"\n";

				return emgData;
			}
		}
	}