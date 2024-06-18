#ifndef _CARPUTER_SERIAL_AGENT_HPP_
#define _CARPUTER_SERIAL_AGENT_HPP_

//#include "ProtocolDefinitions.hpp"
// TODO: Conditional include if no STL
#include "etl/map.h"
#include "etl/deque.h"

extern "C"
{
#include "serial-protocol.pb.h"
}

#ifndef SERIAL_AGENT_INPUT_BUFFER_SIZE 
#define SERIAL_AGENT_INPUT_BUFFER_SIZE (256)
#endif

// The start character of a frame
#ifndef SERIAL_AGENT_FRAME_START
#define SERIAL_AGENT_FRAME_START 'F'
#endif

#ifndef SERIAL_AGENT_FRAME_ESCAPE
#define SERIAL_AGENT_FRAME_ESCAPE '\\'
#endif

#ifndef SERIAL_AGENT_FRAME_END
#define SERIAL_AGENT_FRAME_END 'E'
#endif

namespace Carputer
{

/** @brief The base class for talking back and forth over serial.
 */
class SerialAgent
{
public:
    SerialAgent() {}
    virtual ~SerialAgent();
    /** @brief Alternative to `Handle()`. Called on a raw buffer and does some error
     * checking before calling `Handle()`. */
    bool HandleIncomingMessage(const char *buffer, size_t bufferSize);

    /** @brief Registers `handler` for `MessageType` `type`. */
    void RegisterHandler(MessageType type, MessageHandler handler);

    /** @brief When called, the appropriate handler for `message`'s `type` will be
     * called. If there is not handler registered, returns `false`. */
    bool Handle(const Message &message);

    /** @brief Sends message over the serial line */
    //virtual bool SendMessage(MessageType type, const char* extraData, size_t sizeOfExtraData);

    /** @brief Adds received characters to input buffer.
     * Returns true on success, false on failure (usually buffer is full) */
    bool ReceiveCharacters(const char inputBuffer[], size_t size);

    /** @brief Parses the input buffer and handles any messages it can parse out of there.
     * @returns the number of messages handled */
    size_t SerialAgent::HandleReceivedMessages();

    bool HandleMessage(SimpleMessage &message);

    inline bool IsInputBufferFull() const
    {
        return m_inputBufferIndex == SERIAL_AGENT_INPUT_BUFFER_SIZE;
    }
protected:
    etl::map<MessageType, MessageHandler> m_handlers;

    char m_inputBuffer[SERIAL_AGENT_INPUT_BUFFER_SIZE] {0};

    struct FrameBoundaries
    {
        size_t start;
        size_t end;
    }
    etl::deque<FrameBoundaries> m_foundFrameBoundaries {};

    /** @brief Points to next char to insert.
     * Buffer is full when m_inputBufferIndex == SERIAL_AGENT_INPUT_BUFFER_SIZE */
    size_t m_inputBufferIndex;

    /////////////////////////////////////// Frame marking code

    /** @brief Records the slice from `[startIndex, endIndex)` as a frame.
     * Internally, this means the indices are pushed to a list of indices so when
     * it is time to process the input messages, we can find them easily. */
    void MarkFrameBoundaries(size_t startIndex, size_t endIndex); //TODO
    

    /** Only used in MarkFrame*() functions */
    bool m_hasFoundAFrameStart {false};

    /** @brief the index of the start of a frame currently being read. The index is into m_inputBuffer. */
    size_t m_wipFrameStart {0};

    /** @brief Convenience function to mark the start of a frame in m_inputBuffer. */
    void MarkFrameStart(size_t startIndex);
    /** @brief Convenience function to mark the end of a frame in m_inputBuffer. */
    void MarkFrameEnd(size_t endIndex);

    /////////////////////////////////////// Frame marking code

    bool HasHandlerForType(MessageType type);

};

class SerialClient : public SerialAgent
{
public:
};

}
#endif
