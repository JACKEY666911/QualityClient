#include "messagequeue.h"

#include "qamqpexchange.h"
#include "qamqpqueue.h"
#include "qamqpclient.h"

MessageQueue::MessageQueue(QObject *parent) : QObject(parent) {
  m_client.reset(new QAmqpClient);
  connect(m_client.data(), SIGNAL(connected()), this, SLOT(clientConnected()));
  m_client->setAutoReconnect(true, 3000);
}
MessageQueue::~MessageQueue() { m_client->disconnectFromHost(); }

void MessageQueue::start() {
  if (m_client->isConnected()) {
    m_client->disconnectFromHost();
    m_client->connectToHost();
    return;
  }
  m_client->setHost(m_host);
  m_client->setPort(m_port);
  m_client->setUsername(m_username);
  m_client->setPassword(m_password);
  m_client->connectToHost();
}

QString MessageQueue::host() const { return m_host; }
void MessageQueue::setHost(const QString &value) { m_host = value; }

quint16 MessageQueue::port() const { return m_port; }
void MessageQueue::setPort(quint16 value) { m_port = value; }

QString MessageQueue::username() const { return m_username; }
void MessageQueue::setUsername(const QString &value) { m_username = value; }

QString MessageQueue::password() const { return m_password; }
void MessageQueue::setPassword(const QString &value) { m_password = value; }

QString MessageQueue::queueName() const { return m_queueName; }
void MessageQueue::setQueueName(const QString &value) { m_queueName = value; }

void MessageQueue::clientConnected() {
  QAmqpQueue *temporaryQueue = m_client->createQueue(m_queueName);
  disconnect(temporaryQueue, nullptr, nullptr, nullptr);
  connect(temporaryQueue, SIGNAL(declared()), this, SLOT(queueDeclared()));
  connect(temporaryQueue, SIGNAL(messageReceived()), this,
          SLOT(messageReceived()));
  QAmqpTable args;
  args.insert("x-max-length", 100);
  args.insert("x-expires", 20000);
  temporaryQueue->declare(QAmqpQueue::Durable, args);
}

void MessageQueue::queueDeclared() {
  auto *temporaryQueue = qobject_cast<QAmqpQueue *>(sender());
  if (temporaryQueue == nullptr) {
    return;
  }

  temporaryQueue->consume(QAmqpQueue::coNoAck);

  temporaryQueue->bind("fasys_face_client_exchange", QString());
}

void MessageQueue::messageReceived() {
  auto *temporaryQueue = qobject_cast<QAmqpQueue *>(sender());
  if (temporaryQueue == nullptr) {
    return;
  }

  const QAmqpMessage &message = temporaryQueue->dequeue();
  emit recive(QString::fromUtf8(message.payload()));
  // qDebug() << " [x] " << message.routingKey() << ":" << message.payload();
}

