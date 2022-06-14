from readline import insert_text
import sys
import time
import datetime
from socket import socket, AF_INET, SOCK_STREAM, htons, htonl, ntohs, ntohl, timeout
import threading
import sqlite3

threadLock = threading.Lock()
SNAPSHOT = 0
CLOSE_CONNECTION = 1

def StartServerSocket(ip, port):
    connector = socket(AF_INET, SOCK_STREAM)
    connector.bind((ip,port))
    connector.listen(100)
    
    return connector


class ThreadPackage:
    def __init__(self) -> None:
        self.tid = 0;
        self.parentPid = 0;

    def from_netbytes(self, b):
        self.tid = ntohl(int.from_bytes(b[4:8],sys.byteorder))
        self.parentPid = ntohl(int.from_bytes(b[8:],sys.byteorder))


class ProcessPackage:
    def __init__(self) -> None:
        self.pid = 0
        self.parentPid = 0;
        self.shortName = ""
        self.createPath = ""

    def from_netbytes(self, b):
        self.pid = ntohl(int.from_bytes(b[4:8],sys.byteorder))
        self.parentPid = ntohl(int.from_bytes(b[8:12],sys.byteorder))
        shortName = b[12:28]
        for i,c in enumerate(shortName):
            if c==0:
                self.shortName=shortName[:i].decode()
                break
        createPath_net = b[28:]
        createPath = bytes()
        for i in range(0,len(createPath_net),2):
            createPath += int.to_bytes(ntohs(int.from_bytes(createPath_net[i:i+2],sys.byteorder)),2,sys.byteorder)
        self.createPath = createPath.decode("utf-16")


class ClientHandle(threading.Thread):
    def __init__(self,socketTuple):
        threading.Thread.__init__(self)
        self.SocketTuple = socketTuple
        self.SocketTuple[0].setblocking(True)
        self.SocketTuple[0].settimeout(60)

    def AuthCheck(self, userID, token):
        print("Recieve Auth request: {} - {}".format(userID,token))
        sql = "SELECT UserID from UserTable WHERE UserID=? and Token=?"
        self.databaseCursor.execute(sql,(userID,token))
        return len(self.databaseCursor.fetchall())==1
    
    def AuthGet(self, socket):
        AuthBytes = socket.recv(14)
        if len(AuthBytes)==0:
            raise BrokenPipeError()
        threadLock.acquire()
        print("Recieve Auth Request, request length is {}".format(len(AuthBytes)))
        threadLock.release()
        
        userID = int.from_bytes(AuthBytes[0:4],sys.byteorder)
        token = int.from_bytes(AuthBytes[5:9],sys.byteorder)
        deviceID = int.from_bytes(AuthBytes[10:14],sys.byteorder)
        userID = ntohl(userID)
        token = ntohl(token)
        deviceID = ntohl(deviceID)
        
        threadLock.acquire()
        if self.AuthCheck(userID,token):
            print("Auth Success, DID is:")
            returns = (int.to_bytes(1,1,sys.byteorder), True)
            threadLock.release()
            sql = "SELECT DID FROM DeviceTable WHERE UserID=? and DeviceID=?;"
            self.databaseCursor.execute(sql,(userID,deviceID))
            result = self.databaseCursor.fetchall()
            if(len(result)==0):
                sql = "INSERT INTO DeviceTable (UserID,DeviceID) VALUES (?,?);"
                self.databaseCursor.execute(sql,(userID,deviceID))
                sql = "SELECT DID FROM DeviceTable WHERE UserID=? and DeviceID=?;"
                self.databaseCursor.execute(sql,(userID,deviceID))
                result = self.databaseCursor.fetchall()
            DID = result[0][0]
            print(DID)
        else:
            print("Auth Failed")
            returns = (int.to_bytes(0,1,sys.byteorder), False)
            threadLock.release()
            DID=0        

        socket.send(returns[0])
        return returns[1],userID,token,DID
    
    def SendControl(self,socket,signal):
        if signal==SNAPSHOT:
            socket.send(int.to_bytes(0,1,sys.byteorder))
        elif signal==CLOSE_CONNECTION:
            socket.send(int.to_bytes(1,1,sys.byteorder))
        else:
            return

        state = socket.recv(1)
        if len(state)==0:
            raise BrokenPipeError()
        state = int.from_bytes(state,sys.byteorder)
        if state!=1:
            raise ValueError()
        
        return

    def ReceiveData(self,socket,DID):
        buff = bytes()
        needRead = True
        while(True):
            if needRead:
                buff += socket.recv(1024)
                needRead = False

            # if len(buff) == 0:
            #     print("len of buff is zero")
            #     raise BrokenPipeError()

            if len(buff) < 2:
                needRead = True
                continue

            infoLength = ntohs(int.from_bytes(buff[0:2],sys.byteorder))
            infoType = ntohs(int.from_bytes(buff[2:4],sys.byteorder))

            if infoLength==0:
                if len(buff) < 12:
                    needRead = True
                    continue
                else:
                    if buff[0:12]==int.to_bytes(0,12,sys.byteorder):
                        socket.send(int.to_bytes(1,1,sys.byteorder))
                        self.database.commit()
                        break;
                    else:
                        print("Info transmited is error")
                        socket.send(int.to_bytes(0,1,sys.byteorder))
                        self.database.rollback()
                        break
            
            if len(buff) < infoLength:
                needRead = True
                continue

            if infoType<5 and infoType>=0:
                process = ProcessPackage()
                process.from_netbytes(buff[:infoLength])
                print("ProcessID:{} ParentProcessID:{} shortName:{} CreatePath:{}".format(
                      process.pid,process.parentPid,process.shortName,process.createPath))
                sql = "INSERT INTO ProcessInfoTable (DID,PID,ParentPID,ShortName,CreatePath,InsertDate) VALUES (?,?,?,?,?,?);"
                self.databaseCursor.execute(sql,(DID,process.pid,process.parentPid,process.shortName,process.createPath.encode('utf-8'),str(datetime.datetime.now())))
            # elif infoType==2:
            #     thread = ThreadPackage()
            #     thread.from_netbytes(buff[:infoLength])
            #     print("ThreadID:{} ParentProcessID:{}".format(
            #           thread.tid,thread.parentPid))
            else:
                print("Info transmited is error")
                socket.send(int.to_bytes(0,1,sys.byteorder))
                break
            # check buff and into sqlite3
            buff = buff[infoLength:]

    def run(self):
        clientSocket = self.SocketTuple[0]
        try:
            self.database = sqlite3.connect("./Graduation.db")
            self.databaseCursor = self.database.cursor()
            result,userID,token,DID = self.AuthGet(clientSocket)
            self.database.commit()
            
            if result == False:
                clientSocket.close()
                return False
            
            for i in range(10):
                print("\nNo.{} Round".format(i))
                self.SendControl(clientSocket,SNAPSHOT)
                self.ReceiveData(clientSocket,DID)
                self.database.commit()
                time.sleep(5)
            
            self.SendControl(clientSocket,CLOSE_CONNECTION)
        except BrokenPipeError as e:
            print("Connection lost: BrokenPipeError")
        except timeout as e:
            print("Connection lost: timeout",e)
        except OSError as e:
            print("Connection lost: OSError",e)
        except ValueError as e:
            print("Protocol error: ValueError",e)
        except KeyboardInterrupt as e:
            print("Closed by remote user: KeyboardInterrupt",e)
        finally:
            clientSocket.close()
            self.databaseCursor.close()
            self.database.close()


if __name__ == '__main__':
    threadPool = list()
    try:
        serverSocket = StartServerSocket("0.0.0.0",5555);
        try:
            while(True):
                client = serverSocket.accept()
                
                one_thread = ClientHandle(client)
                one_thread.start()
                threadPool.append(one_thread)
                
                for i in threadPool:
                    if i.is_alive():
                        continue
                    else:
                        i.join(1)
                        threadPool.remove(i)
        except KeyboardInterrupt as e:
            print("Main: Closed by remote user")
        finally:
            for i in threadPool:
                i.SocketTuple[0].close()
            serverSocket.close()
    except Exception as e:
        print("Start server error, ip or port wrong")


