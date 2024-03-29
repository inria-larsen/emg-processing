#!/usr/bin/env python
# license removed for brevity
import rospy
from std_msgs.msg import String  
from std_msgs.msg import Float64

import socket
import sys
import struct

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to localhost
server_address = ("127.0.0.1", 10501)
print >>sys.stderr, 'starting up on %s port %s' % server_address
sock.bind(server_address)
# Listen for incoming connections
sock.listen(1)


# def talker():
#     pub = rospy.Publisher('chatter', String, queue_size=10)
#     rospy.init_node('talker', anonymous=True)
#     rate = rospy.Rate(10) # 10hz
#     while not rospy.is_shutdown():
#         hello_str = "hello world %s" % rospy.get_time()
#         rospy.loginfo(hello_str)
#         pub.publish(hello_str)
#         rate.sleep()

if __name__ == '__main__':
    # connection
    try: 
        myMsg = Float64()
        pub = rospy.Publisher('icc_1', Float64, queue_size=10)
        # pub2 = rospy.Publisher('icc_2', Float64, queue_size=10)
        rospy.init_node('emgNode')
        rate = rospy.Rate(1000) # 1000hz
        connStatus = True
        while (not rospy.is_shutdown()) and connStatus:
            # Wait for a connection
            print >>sys.stderr, 'waiting for a connection'
            connection, client_address = sock.accept()
            try:
                print >>sys.stderr, 'connection from', client_address
                # break
                print("This will receive icc signal and broadcast it to the ROS network")
                # # Receive the data in small chunks and retransmit to ROS
                while not rospy.is_shutdown():
                    # pub.publish(myMsg)
                    rate.sleep()
                    # pass
                    rosData = connection.recv(1024)
                    # rosData = struct.unpack('<1f',data)
                    # myMsg.data = rosData
                    # print(rosData)
                    if rosData:
                        splitMsg = rosData.split(',')
                        rosDataFloat = float(splitMsg[0])
                        myMsg = rosDataFloat
                        # print >>sys.stderr, 'publishing icc to ROS'
                        pub.publish(myMsg)
                        # if(splitMsg[1]):
                        #     myMsg = float(splitMsg[1])
                        #     # pub2.publish(myMsg)
                            
                    else:
                        break
                        # connection.sendall(data)
                #     # else:
                #     #     print >>sys.stderr, 'no more data from', client_address
                #     #     break
            except:
                # Clean up the connection
                print("Cleaning up connection from **exception**")
                connection.close()
            finally:
                # Clean up the connection
                print("Cleaning up connection from **finally**")
                connection.close()
                connStatus = False

    except rospy.ROSInterruptException:
        print("Caught ROS interruption")

    sock.close()

