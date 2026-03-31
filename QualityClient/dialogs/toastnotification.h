#ifndef TOASTNOTIFICATION_H
#define TOASTNOTIFICATION_H

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

class ToastNotification : public QFrame {
  Q_OBJECT
 public:
  enum Type { Info, Success, Warning, Error };
  explicit ToastNotification(QWidget* parent = nullptr);

  static void showToast(QWidget* parent, const QString& message,
                        Type type = Info, int durationMs = 3000);

 protected:
  void applyTypeStyle(Type type);
  void paintEvent(QPaintEvent* event) override;

 private:
  QLabel* m_iconLabel;
  QLabel* m_messageLabel;
};

#endif