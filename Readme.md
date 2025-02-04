# IRC

Arguments of the program :
    - port: the listening port
    - password: the connection password

client to use: irssi : https://irssi.org/
documentation for RPL code : https://www.alien.net.au/irc/irc2numerics.html

example to run the program:

./ircserv \<port\> \<password\>

## Requirement

- The server must hande multiple client and never hang
- forking is not allowed, everything must be non blocking
- only 1 poll() (or equivalent) can be used for all these operations (read, write, listen,...)
- The client reference that we choose is going to be used during the correction
- the client reference must be able to connect to your server without encountering any error
- communication has to be done via TCP/IP (v4 or v6)

## Features to implement :

    - [ ] you must be able to authenticate, set a nickname, a username, join a channel
          send and receive private message using the reference client
    - [ ] the message must be send to all other client in the same channel
    - [ ] we must have operators and regular users
    - [ ] implement command specific to a channel operator :
            - [ ] KICK : eject a client from the channel
            - [ ] INVITE : invite a client to a channel
            - [ ] TOPIC : change or view the channel topic
            - [ ] MODE : change the channel's mode to :
                - [ ] i : set/remove invite-only channel
                - [ ] t : set/remove the restrictions of the TOPIC command to channel operators
                - [ ] k : set/remove the channel key (password)
                - [ ] o : give/take channel operator privilege
                - [ ] l : set/remove the user limit to channel 

    ctrl + D is use to send command in several parts:

    ex : 
        $> nc 127.0.0.1 6667
        com^Dman^Dd
        $>
    
    = 'com' , then 'man' , then 'd\n'
