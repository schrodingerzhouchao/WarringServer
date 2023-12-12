See the diagrams about various parts of the project.

# Base

## message

### BaseMessage
    Message structure for network transmission, supports serialization and deserialization of primitive types，std::string, std::vector<primitive type> and std::vector<std::string> for now.

### CustomMessage
    Inherit from BaseMessage.

## net
    Based on Reactor model, one event loop per thread and thread pool. Acceptor as main reactor which accept new connections and register connected connfds to sub reactor(EventloopThreadpool).

## utility

### TimewheelTimer
    Timewheel is similar to normal clock including second, minute and hour slots. Each slot contains list of tasks which registered by other threads. Check slots in sequence per second, and invoke task in this slot if which is timeout.

### Logger
    Multithread asynchronous logger. Each thread bind with one LoggerLine, so no need mutex to protect each write. There are two queue: ReadyBufferQueue and EmptyBufferQueue: ReadyBufferQueue accepts each LoggerLine's Buffer's pointer if this Buffer is full or timeout; EmptyBufferQueue accepts LoggerLine's Buffer's pointer which has finished archive, and notifies LoggerLine there is an empty Buffer.

# Broker

## BrokerHelper
    About message queue methods based on Redis Stream. 

## BrokerManager
    Check whether message is pending and decides to deliver, ack or delete it.
