#include "SerialAgent.hpp"
extern "C"{
#include <pb_encode.h>
#include <pb_decode.h>

#include "serial-protocol.pb.h"
}

namespace Carputer
{

SerialAgent::~SerialAgent()
{
}

bool SerialAgent::ReceiveCharacters(const char inputBuffer[], size_t size)
{
    size_t index = 0;
    size_t framesInBuffer = 0;
    size_t numEscapeCharacters = 0;
    bool inEscapeMode = false;

    while (!IsInputBufferFull() && index < size)
    {
        bool addThisChar = false;
        // Strip escapes
        char inputChar = inputBuffer[index];
        if (inputChar == SERIAL_AGENT_FRAME_ESCAPE)
        {
            // If we are already in escape mode, this is an original escape char in the data
            if (inEscapeMode)
            {
                addThisChar = true;
                inEscapeMode = false;
            }
            else
            {
                // Not already in escape mode; turn escape on and skip this char.
                inEscapeMode = true;
            }
        }
        else if (inputChar == SERIAL_AGENT_FRAME_START)
        {
            if (inEscapeMode)
            {
                addThisChar = true;
                inEscapeMode = false;
            }
            else
            {
                // Not already in escape mode; turn escape on and skip this char.
                MarkFrameStart(m_inputBufferIndex);
            }
        }
        else if (inputChar == SERIAL_AGENT_FRAME_END)
        {
            if (inEscapeMode) // Genuine data, put it in.
            {
                addThisChar = true;
                inEscapeMode = false;
            }
            else
            {
                MarkFrameEnd(m_inputBufferIndex + 1);
            }
        }
        else // A regular, not special character
        {
            addThisChar = true;
        }


        if (addThisChar)
        {
            m_inputBuffer[m_inputBufferIndex] = inputBuffer[index];
            ++m_inputBufferIndex;
        }
        ++index;
    }
}

void SerialAgent::MarkFrameStart(size_t startIndex)
{
    if (m_hasFoundAFrameStart)
    {
        // TODO: Log that an unexpected thing happened
    }
    m_hasFoundAFrameStart = true;
    m_wipFrameStart = startIndex;
}

void SerialAgent::MarkFrameEnd(size_t endIndex)
{
    if (!m_hasFoundAFrameStart)
    {
        // TODO: Log an error
        return;
    }

    MarkFrameBoundaries(m_wipFrameStart, endIndex);

    m_wipFrameStart = 0;
    m_hasFoundAFrameStart = false;
}

void MarkFrameBoundaries(size_t startIndex, size_t endIndex)
{
    FrameBoundaries frame;
    frame.start = startIndex;
    frame.end = endIndex;
    m_foundFrameBoundaries.push_back(frame);
}

// This function is fraught with the possibility for off-by-one errors.
size_t SerialAgent::HandleReceivedMessages()
{
    size_t bytesConsumed = 0;

    // For each found frame boundary,
    for (; !m_foundFrameBoundaries.empty(); m_foundFrameBoundaries.pop_front())
    {
        // Validate the frame.
        if (IsFrameValid(m_foundFrameBoundaries.front()))
        {
            // Reconstruct the message
            // // TODO

            // Run the handlers for the message.
        }
        else // If the frame is not valid, discard.
        {
            // TODO: Log an error
        }
    }
    //
    // Copy tail end of buffer to beginning of buffer
    // Fix m_foundFrameBoundaries entries.
    // Fix m_wipFrameStart
#if 0
    size_t messagesHandled = 0;
    // Make an input stream from the input buffer
    pb_istream_t inStream = pb_istream_from_buffer(m_inputBuffer, m_inputBufferIndex);
    bool parseResult = false;
    while (true)
    {
        Serial.println("m_inputBuffer = " + String((uint32_t)m_inputBuffer));
        Serial.println("m_inputBufferIndex = " + String(m_inputBufferIndex));
        SimpleMessage parsedMessage;
        parseResult = pb_decode_ex(&inStream, SimpleMessage_fields, &parsedMessage, PB_DECODE_DELIMITED | PB_DECODE_NULLTERMINATED);
        if (parseResult == false)
        {
            Serial.println("Decoding failed. Breaking now.");
            break;
        }
        if(HandleMessage(parsedMessage))
        {
            messagesHandled++;
        }
        else
        {
            Serial.println("Could not handle message for some reason.");
        }
    }
    
    size_t consumedBytes = m_inputBufferIndex - inStream.bytes_left;
    // The number of bytes consumed is also the position of the first unconsumed byte

    // Copy all the useful data over to the start of the buffer
    // TODO: Fix all the "Found frame" indices in m_foundFrameBoundaries
    // TODO: Maybe consume as much as you can out of the buffer first.
    size_t endOfBufferData = m_inputBufferIndex;
    for (size_t i=0; i < endOfBufferData - consumedBytes; ++i)
    {
        m_inputBuffer[i] = m_inputBuffer[consumedBytes + i];
    }

    // Clear the copied data at the end of the buffer.
    memset(&m_inputBuffer[endOfBufferData - consumedBytes], '\0', consumedBytes);

    // Set m_inputBufferIndex to the right thing now.
    m_inputBufferIndex = endOfBufferData - consumedBytes;

    return messagesHandled;
#endif
}

bool SerialAgent::HandleMessage(SimpleMessage &message)
{
    // TODO: Find the message type and switch on that.
    Serial.println("SerialAgent::HandleMessage(): Got message");
#if 0
    Serial.println("--> has_header: " + String(message.has_header));
    Serial.println("--> which_request: " + String(message.which_request));
    Serial.println("--> which_response: " + String(message.which_response));
#endif
    return true;
}

bool SerialAgent::HandleIncomingMessage(const char *buffer, size_t bufferSize)
{
    pb_istream_t stream = pb_istream_from_buffer(buffer, bufferSize);

    SerialMessage parsedMessage = {};

    if (!pb_decode(&stream, SerialMessage_fields, &parsedMessage))
    {
        // Throw error
        // TODO: Move this to a logger class
        Serial.println("--> ERROR: Could not decode message!");
        return false;
    }
    Serial.print("--> Got message. whichRequest = ");
    Serial.print(parsedMessage.which_request);
    Serial.print(", which_response = ");
    Serial.println(parsedMessage.which_response);

    if (parsedMessage.which_request)
    {
    }
    else if (parsedMessage.which_response)
    {
    }

    return true;
}

void SerialAgent::RegisterHandler(MessageType type, MessageHandler handler)
{
    // Warn if about to overwrite
    if (HasHandlerForType(type))
    {
        Serial.print("WARNING: about to replace the handler for message type ");
        Serial.println(type);
    }
    m_handlers[type] = handler;
}

bool SerialAgent::Handle(const Message &message)
{
    if (!HasHandlerForType(message.type))
    {
        return false;
    }
    m_handlers[message.type](message);
    return true;
}

bool SerialAgent::HasHandlerForType(MessageType type)
{
    return m_handlers.find(type) != m_handlers.end();
}

} /* namespace Carputer */
