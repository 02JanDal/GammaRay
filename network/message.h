#ifndef GAMMARAY_MESSAGE_H
#define GAMMARAY_MESSAGE_H

#include "protocol.h"

#include <QByteArray>
#include <QDataStream>

namespace GammaRay {

/**
 * Single message send between client and server.
 * Binary format:
 * - sizeof(Protocol::PayloadSize) byte size of the message payload (not including the size and other fixed fields itself) in netowork byte order (big endian)
 * - sizeof(Protocol::ObjectAddress) server object address (big endian)
 * - sizeof(Protocol::MessageType) command type (big endian)
 * - size bytes message payload (encoding is user defined, QDataStream provided for convenience)
 */
class Message
{
  public:
    /**
     * Construct a new message to/from @p address and message type @p type.
     */
    explicit Message(Protocol::ObjectAddress address, Protocol::MessageType type);
#ifdef Q_COMPILER_RVALUE_REFS
    Message(Message &&other);
#else
    // this is only needed to make readMessage compile (due to RVO there is no actual copy though)
    // semantically we don't want to support copying, due to the datastream state
    Message(const Message &other);
#endif
    ~Message();

    Protocol::ObjectAddress address() const;
    Protocol::MessageType type() const;

    /** Access to the message payload. This is read-only for received messages
     *  and write-only for messages to be sent.
     */
    QDataStream& payload() const;

    /** Checks if there is a full message waiting in @p device. */
    static bool canReadMessage(QIODevice *device);
    /** Read the next message from @p device. */
    static Message readMessage(QIODevice *device);

    /** Write this message to @p device. */
    void write(QIODevice *device) const;

  private:
    Message();

    mutable QByteArray m_buffer;
    mutable QScopedPointer<QDataStream> m_stream;

    Protocol::ObjectAddress m_objectAddress;
    Protocol::MessageType m_messageType;
};

}

#endif