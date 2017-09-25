/*
 * Emg Tcp
 *
 * Author: Waldez Gomes
 * email:  waldezjr14@gmail.com
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/


#ifndef EMG_TCP_H
#define EMG_TCP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <yarp/os/all.h>


#define SZ_DATA_IM_EMG  (64)

/**
 * @brief      Contains the RAW signals from the 16 IM EMG sensors at 'timeStamp'
 */
struct EmgData{
	public:
		/**
		 * TimeStamp - NOT IMPLEMENTED YET
		 */
		double timeStamp = 0;
		/**
		 * serialized raw data from the 16 EMG sensors
		 */
		std::vector<float> data;

};

/**
 * @brief      Class for interface with the Delsys SDK TCP server
 */
class EmgTcp{

	public:
	
		EmgTcp();
		/**
		 * @brief      Valid Constructor
		 *
		 * @param[in]  servIpAdd  The serv ip add
		 */
		EmgTcp(std::string servIpAdd);
		/**
		 * @brief      Determines if command connected.
		 *
		 * @return     True if command connected, False otherwise.
		 */
        bool isCmdConnected(void);
        /**
         * @brief      Determines if im emg connected.
         *
         * @return     True if im emg connected, False otherwise.
         */
        bool isImEmgConnected(void);
        /**
         * @brief      Determines if streaming.
         *
         * @return     True if streaming, False otherwise.
         */
        bool isStreaming(void);
		/**
		 * @brief      Sets the ip add.
		 *
		 * @param[in]  servIpAdd  The serv ip add
		 */
		void setIpAdd(const std::string servIpAdd);
		/**
		 * @brief      Connects to the Delsys SDK TCP server.
		 */
		bool connect2Server();
		/**
		 * @brief      Send configuration commands to the command port of the SDK TCP server
		 */
		void configServer();
		/**
		 * @brief      Starts a data stream. (For now only from the IM EMG sensors)
		 */
		void startDataStream();
		/**
		 * @brief      Stops a data stream. (For now only from the IM EMG sensors)
		 */
		void stopDataStream();
		/**
		 * @brief      Gets the EmgData.
		 *
		 * @return     The EmgData now.
		 */
		EmgData getData();



	private:
		/**
		 * @brief      Connects to the command port
		 */
		bool connectCmdTcp(void);
        /**
         * @brief      Closes a command sock.
         */
        void closeCmdSock(void);
		/**
		 * @brief      Connects to im emg data port.
		 */
		bool connectImEmgTcp(void);
		/**
		 * @brief      Closes an im emg data sock.
		 */
        void closeImEmgSock(void);
        /**
         * @brief      Resolves hostname
         *
         * @param[in]  host  The host
         * @param      addr  The address
         *
         * @return     hostname}
         */
        int resolveHostName(const char* host, struct in_addr* addr);
		/**
		 * @brief      Sends a command to the SDK TCP command port and waits for reply
		 *
		 * @param[in]  cmd   The command
		 *
		 * @return     The reply
		 */
		std::string sendCmd(std::string cmd);
		/**
		 * @brief      Writes a command into TCP socket
		 *
		 * @param[in]  cmd   The command string
		 */
		int writeCmd(std::string cmd); 

		/**
		 * @brief      Reads a command reply from TCP socket
		 *
		 * @return     Command reply string
		 */
		std::shared_ptr<std::string>  readCmdReply();
        

        char imEmgData_[SZ_DATA_IM_EMG];
        struct sockaddr_in addr_;

        std::string servIpAdd_;
        int cmdPort_;
        int imEmgPort_;
        int imEmgSock_;
        int cmdSock_;
        bool cmdConnected_ = false;
        bool imEmgConnected_ = false;
        bool streamingData_ = false;



};


#endif


