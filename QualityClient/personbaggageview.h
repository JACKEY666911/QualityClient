#ifndef PERSONBAGGAGEVIEW_H
#define PERSONBAGGAGEVIEW_H

#include <QWidget>

namespace Ui {
class PersonBaggageView;
}

class PersonBaggageView : public QWidget
{
    Q_OBJECT

public:
    explicit PersonBaggageView(QWidget *parent = nullptr);
    ~PersonBaggageView();
    void clearViewData();

signals:
    void requestBack();

private slots:
    void on_videoBtn_clicked();

    void on_prevPassBtn_clicked();

    void on_nextPassBtn_clicked();

    void on_prevBagBtn_clicked();

    void on_nextBagBtn_clicked();

    void on_backButton_clicked();

private:
    Ui::PersonBaggageView *ui;
};

#endif // PERSONBAGGAGEVIEW_H
