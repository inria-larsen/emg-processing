The template file is going to be installed in your local installation folder for applications/scripts.
Everytime you do a ``git pull && cd build && make && make install`` it will be overwritten. 

Make a copy of the template file, rename it with the XML extension. 
Modify the file to make it compliant to your installation.

For example, you probably need to change the name of the computation nodes (by default here ``andyNode``).
If you do not know what a computation node is in yarp, it is a yarprun node that was created to execute programs via yarpmanager. If you don't have a cluster installtion, simply do:

```
yarp namespace /testEMG
yarpserver
yarprun --server /andyNode --log
```

to create a yarpserver with a node called /andyNode. 

To execute the XML file, simply open 

```
yarpmanager
```

If you wish to read the modules' logs, you should also run yarplogger:

```
yarplogger
```

then import the XML file. Use the Play/Run button to execute the modules, then click on "Connect" to connect the ports.



