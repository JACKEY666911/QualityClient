#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include "qamqpclient.h"
#include "qamqpqueue.h"
#include <QObject>
#include <QString>

class MessageQueue : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
  Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY portChanged)
  Q_PROPERTY(
      QString username READ username WRITE setUsername NOTIFY usernameChanged)
  Q_PROPERTY(
      QString password READ password WRITE setPassword NOTIFY passwordChanged)
  Q_PROPERTY(QString queueName READ queueName WRITE setQueueName NOTIFY
                 queueNameChanged)
 public:
  explicit MessageQueue(QObject *parent = nullptr);
  ~MessageQueue() override;
  Q_INVOKABLE void start();
  [[nodiscard]] QString host() const;
  void setHost(const QString &value);
  [[nodiscard]] quint16 port() const;
  void setPort(quint16 value);
  [[nodiscard]] QString username() const;
  void setUsername(const QString &value);
  [[nodiscard]] QString password() const;
  void setPassword(const QString &value);
  [[nodiscard]] QString queueName() const;
  void setQueueName(const QString &value);

 signals:
  void recive(QString msg);
  void hostChanged(QString value);
  void portChanged(quint16 value);
  void usernameChanged(QString value);
  void passwordChanged(QString value);
  void queueNameChanged(QString value);

 private slots:
  void clientConnected();
  void queueDeclared();
  void messageReceived();

 private:
  QScopedPointer<QAmqpClient> m_client;
  QString m_host;
  quint16 m_port;
  QString m_username;
  QString m_password;
  QString m_queueName;
};

#endif // MESSAGEQUEUE_H
