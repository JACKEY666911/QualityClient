#ifndef HISTORYMAINVIEWWIDGET_H
#define HISTORYMAINVIEWWIDGET_H

#include <QWidget>
#include <QList>
#include <QDateTime>

#include "Models/ImageDistributeInfo.h"

class QLabel;
class QPushButton;
class QListView;
class QStandardItemModel;
class QualityCardDelegate;
class QComboBox;
class QLineEdit;
class QDateTimeEdit;

class HistoryMainViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryMainViewWidget(QWidget *parent = nullptr);
    ~HistoryMainViewWidget() override;
    void setUserName(const QString &userName);
    void setHistoryInfos(const QList<ImageDistributeInfo> &infos);
    void clearHistoryInfos();

signals:
    void requestBack();
    void historyDetailRequested(const ImageDistributeInfo &info);

private:
    void buildUi();
    void applyFilters();
    static QDateTime parseTaskTime(const ImageDistributeInfo &info);
    static bool hasRenderableImage(const ImageDistributeInfo &info);

    QLabel *m_pageTitleLabel;
    QPushButton *m_backButtonTop;
    QLabel *m_userNameLabel;
    QLineEdit *m_channelEdit;
    QComboBox *m_imageTypeCombo;
    QComboBox *m_aiResultCombo;
    QComboBox *m_channelJudgeCombo;
    QDateTimeEdit *m_startTimeEdit;
    QDateTimeEdit *m_endTimeEdit;
    QLineEdit *m_qcUserEdit;
    QComboBox *m_qcResultCombo;
    QPushButton *m_queryButton;
    QPushButton *m_resetButton;
    QListView *m_listView;
    QStandardItemModel *m_model;
    QualityCardDelegate *m_delegate;
    QList<ImageDistributeInfo> m_sourceItems;
    QList<ImageDistributeInfo> m_items;
};

#endif // HISTORYMAINVIEWWIDGET_H
