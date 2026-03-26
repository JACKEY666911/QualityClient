#ifndef APPMESSAGEDIALOG_H
#define APPMESSAGEDIALOG_H

#include <QDialog>

class AppMessageDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AppMessageDialog(const QString &title,
                              const QString &message,
                              QWidget *parent = nullptr);

    static void showInfo(QWidget *parent,
                         const QString &title,
                         const QString &message);
};

#endif // APPMESSAGEDIALOG_H
