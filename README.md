# Protocol Analyzer
## Written in C + LUA + Python

#### :shipit: Analyzer is a project made to detect and analyse a specifc protocol in a pcap file . :shipit:


### How to run the code :
* [**Step 0**] - In order to run this program you need to add the dissector of "ROY" to your wireshark configuration :
```
Open WireShark -> Help -> About Wireshark -> Folders -> Global Configuration -> copy test.lua to the folder
```
You can find the test.lua file which indicates on the way to dissect the protocol in :
```
NewClient -> NewClient -> test.lua
```
* [**Step 1**] - The C Client is responsible for sending a descriptor of the LUA file to the C server.

* [**Step 2**] - The C Client sends random PDU packets that can be either random messages or by the characteristics of "ROY protocol".

* [**Step 3**] - The C Server is responsible of adding every packet it got to a newly created pcap file and sends that and the lua descriptor forward to the Python Server.

* [**Step 4**] - The Python Server is analysing the pcap file and creates a new file named **fixed.pcap** which will hold only the correct packets of protocol **"ROY"**.


### Todos :

 - Edit Client and Server C files for better return checks in main function.
 - Search for a decent Machine Learning Algorithm for pcap analysis.
 - Send back the result of the algorithm to the C Server to process it back to the C Client.
 - Add a GUI output on the C Client for the returned analysis.
  
  
                                                                                                                  
                
 
