#include "HistoryMainViewWidget.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPixmap>
#include <QPushButton>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include "Models/GlobalEnums.h"
#include "Models/QueryInfo.h"
#include "logging/logcategories.h"
#include "widgets/QualityCardDelegate.h"

namespace {
QueryInfo* createCardData(const ImageDistributeInfo& info) {
  QueryInfo* card = new QueryInfo();
  card->id = info.id;
  card->type = info.type;
  card->channelNo = info.channelNo;
  card->channelNoValue = info.channelNoValue;
  card->imageResult = info.imageResult;
  card->qualityResult = info.qualityResult.toInt();
  card->qualityTime = info.qualityControlTime.isEmpty()
                          ? info.checkTime
                          : info.qualityControlTime;
  card->recheckResult = info.recheckResult.toBool();
  if (!info.pbXrayImages.isEmpty()) {
    card->xrayImage = info.pbXrayImages.first();
  } else if (!info.xrayImages.isEmpty()) {
    card->xrayImage = info.xrayImages.first();
  }
  return card;
}

QString channelTextOf(int res) {
  switch (res) {
    case QualityImageResult::StartCheck:
      return QStringLiteral("通道开检");
    case QualityImageResult::EndCheck:
      return QStringLiteral("通道放行");
    default:
      return QStringLiteral("通道未知");
  }
}

QString qualityTextOf(int res) {
  switch (res) {
    case QualityImageResult::StartCheck:
      return QStringLiteral("质控开检");
    case QualityImageResult::EndCheck:
      return QStringLiteral("质控放行");
    default:
      return QStringLiteral("质控未知");
  }
}
}  // namespace

HistoryMainViewWidget::HistoryMainViewWidget(QWidget* parent)
    : QWidget(parent),
      m_pageTitleLabel(nullptr),
      m_backButtonTop(nullptr),
      m_userNameLabel(nullptr),
      m_channelEdit(nullptr),
      m_imageTypeCombo(nullptr),
      m_aiResultCombo(nullptr),
      m_channelJudgeCombo(nullptr),
      m_startTimeEdit(nullptr),
      m_endTimeEdit(nullptr),
      m_qcUserEdit(nullptr),
      m_qcResultCombo(nullptr),
      m_queryButton(nullptr),
      m_resetButton(nullptr),
      m_listView(nullptr),
      m_model(nullptr),
      m_delegate(nullptr) {
  buildUi();
}

HistoryMainViewWidget::~HistoryMainViewWidget() { clearHistoryInfos(); }

void HistoryMainViewWidget::setUserName(const QString& userName) {
  if (m_userNameLabel) {
    m_userNameLabel->setText(userName);
  }
}

void HistoryMainViewWidget::buildUi() {
  setObjectName(QStringLiteral("historyMainView"));
  setStyleSheet(QStringLiteral(
      "QWidget#historyMainView{background:#efefef;}"
      "QLineEdit,QComboBox,QDateTimeEdit{background:#f6f6f6;border:1px solid "
      "#9aa4ab;border-radius:3px;padding:3px 8px;min-height:30px;}"
      "QPushButton#queryBtn{background:#12a9df;color:white;border:none;border-"
      "radius:4px;padding:6px 18px;font:16px \"Microsoft YaHei\";}"
      "QPushButton#resetBtn{background:#f2f2f2;color:#2b2b2b;border:1px solid "
      "#9aa4ab;border-radius:4px;padding:6px 18px;font:16px \"Microsoft "
      "YaHei\";}"));

  QVBoxLayout* root = new QVBoxLayout(this);
  root->setContentsMargins(0, 0, 0, 0);
  root->setSpacing(0);

  QWidget* brandBar = new QWidget(this);
  brandBar->setMinimumHeight(55);
  brandBar->setMaximumHeight(55);
  brandBar->setObjectName(QStringLiteral("historyMainTopBar"));
  brandBar->setStyleSheet(QStringLiteral(
      "QWidget#historyMainTopBar{background:#117595;color:white;}"
      "QLabel{color:white;font-family:\"Microsoft YaHei\";}"
      "QPushButton{background:#12a9df;color:white;border:none;border-radius:"
      "4px;padding:4px 18px;font-family:\"Microsoft YaHei\";font-size:18px;}"));

  QHBoxLayout* brandLayout = new QHBoxLayout(brandBar);
  brandLayout->setContentsMargins(12, 0, 12, 0);
  brandLayout->setSpacing(10);

  QLabel* logoLabel = new QLabel(brandBar);
  logoLabel->setFixedSize(58, 40);
  logoLabel->setPixmap(
      QPixmap(QStringLiteral(":/Images/logo.png"))
          .scaled(58, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  logoLabel->setAlignment(Qt::AlignCenter);

  QLabel* systemTitle =
      new QLabel(QStringLiteral("般睿安检集中质控系统"), brandBar);
  systemTitle->setStyleSheet(
      QStringLiteral("font-size:42px;font-weight:bold;"));

  m_backButtonTop = new QPushButton(QStringLiteral("返    回"), brandBar);
  m_userNameLabel = new QLabel(QStringLiteral("zy"), brandBar);
  m_userNameLabel->setStyleSheet(
      QStringLiteral("font-size:28px;font-weight:500;"));

  brandLayout->addWidget(logoLabel);
  brandLayout->addWidget(systemTitle);
  brandLayout->addStretch(1);
  brandLayout->addWidget(m_backButtonTop);
  brandLayout->addSpacing(12);
  brandLayout->addWidget(m_userNameLabel);
  connect(m_backButtonTop, &QPushButton::clicked, this,
          &HistoryMainViewWidget::requestBack);

  QWidget* searchPanel = new QWidget(this);
  searchPanel->setStyleSheet(QStringLiteral("background:#efefef;"));
  QVBoxLayout* searchRoot = new QVBoxLayout(searchPanel);
  searchRoot->setContentsMargins(14, 10, 14, 8);
  searchRoot->setSpacing(8);

  m_pageTitleLabel = new QLabel(QStringLiteral("质控记录回查"), searchPanel);
  m_pageTitleLabel->setStyleSheet(QStringLiteral(
      "font:48px \"Microsoft YaHei\";font-weight:bold;color:#1f2a35;"));
  searchRoot->addWidget(m_pageTitleLabel);

  QWidget* row1 = new QWidget(searchPanel);
  QHBoxLayout* row1Layout = new QHBoxLayout(row1);
  row1Layout->setContentsMargins(0, 0, 0, 0);
  row1Layout->setSpacing(12);
  QLabel* channelLabel = new QLabel(QStringLiteral("安检通道："), row1);
  channelLabel->setStyleSheet(
      QStringLiteral("font:34px \"Microsoft YaHei\";font-weight:bold;"));
  m_channelEdit = new QLineEdit(row1);
  m_channelEdit->setPlaceholderText(QStringLiteral("8号通道, LJ09"));
  m_channelEdit->setMinimumWidth(280);

  QLabel* imageTypeLabel = new QLabel(QStringLiteral("图像类型："), row1);
  imageTypeLabel->setStyleSheet(channelLabel->styleSheet());
  m_imageTypeCombo = new QComboBox(row1);
  m_imageTypeCombo->addItems({QStringLiteral("全部"), QStringLiteral("原图"),
                              QStringLiteral("增强图")});
  m_imageTypeCombo->setMinimumWidth(190);

  QLabel* aiLabel = new QLabel(QStringLiteral("AI判图结论："), row1);
  aiLabel->setStyleSheet(channelLabel->styleSheet());
  m_aiResultCombo = new QComboBox(row1);
  m_aiResultCombo->addItems({QStringLiteral("全部"), QStringLiteral("通道放行"),
                             QStringLiteral("通道开检"),
                             QStringLiteral("通道未知")});
  m_aiResultCombo->setMinimumWidth(190);

  QLabel* channelJudgeLabel =
      new QLabel(QStringLiteral("通道判图结论："), row1);
  channelJudgeLabel->setStyleSheet(channelLabel->styleSheet());
  m_channelJudgeCombo = new QComboBox(row1);
  m_channelJudgeCombo->addItems(
      {QStringLiteral("全部"), QStringLiteral("通道放行"),
       QStringLiteral("通道开检"), QStringLiteral("通道未知")});
  m_channelJudgeCombo->setMinimumWidth(190);

  m_queryButton = new QPushButton(QStringLiteral("查    询"), row1);
  m_queryButton->setObjectName(QStringLiteral("queryBtn"));
  m_queryButton->setMinimumWidth(120);
  connect(m_queryButton, &QPushButton::clicked, this,
          &HistoryMainViewWidget::applyFilters);

  row1Layout->addWidget(channelLabel);
  row1Layout->addWidget(m_channelEdit);
  row1Layout->addSpacing(10);
  row1Layout->addWidget(imageTypeLabel);
  row1Layout->addWidget(m_imageTypeCombo);
  row1Layout->addSpacing(10);
  row1Layout->addWidget(aiLabel);
  row1Layout->addWidget(m_aiResultCombo);
  row1Layout->addSpacing(10);
  row1Layout->addWidget(channelJudgeLabel);
  row1Layout->addWidget(m_channelJudgeCombo);
  row1Layout->addSpacing(8);
  row1Layout->addWidget(m_queryButton);
  searchRoot->addWidget(row1);

  QWidget* row2 = new QWidget(searchPanel);
  QHBoxLayout* row2Layout = new QHBoxLayout(row2);
  row2Layout->setContentsMargins(0, 0, 0, 0);
  row2Layout->setSpacing(12);
  QLabel* timeLabel = new QLabel(QStringLiteral("质控时间："), row2);
  timeLabel->setStyleSheet(channelLabel->styleSheet());
  m_startTimeEdit =
      new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), row2);
  m_startTimeEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
  m_startTimeEdit->setMinimumWidth(210);
  QLabel* toLabel = new QLabel(QStringLiteral("至"), row2);
  toLabel->setStyleSheet(channelLabel->styleSheet());
  m_endTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), row2);
  m_endTimeEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
  m_endTimeEdit->setMinimumWidth(210);

  QLabel* qcUserLabel = new QLabel(QStringLiteral("质控员："), row2);
  qcUserLabel->setStyleSheet(channelLabel->styleSheet());
  m_qcUserEdit = new QLineEdit(row2);
  m_qcUserEdit->setPlaceholderText(QStringLiteral("zy"));
  m_qcUserEdit->setMinimumWidth(150);

  QLabel* qcResultLabel = new QLabel(QStringLiteral("质控判图结论："), row2);
  qcResultLabel->setStyleSheet(channelLabel->styleSheet());
  m_qcResultCombo = new QComboBox(row2);
  m_qcResultCombo->addItems({QStringLiteral("全部"), QStringLiteral("质控放行"),
                             QStringLiteral("质控开检"),
                             QStringLiteral("质控未知")});
  m_qcResultCombo->setMinimumWidth(190);

  m_resetButton = new QPushButton(QStringLiteral("重    置"), row2);
  m_resetButton->setObjectName(QStringLiteral("resetBtn"));
  m_resetButton->setMinimumWidth(110);
  connect(m_resetButton, &QPushButton::clicked, this, [this]() {
    m_channelEdit->clear();
    m_imageTypeCombo->setCurrentIndex(0);
    m_aiResultCombo->setCurrentIndex(0);
    m_channelJudgeCombo->setCurrentIndex(0);
    m_qcResultCombo->setCurrentIndex(0);
    m_qcUserEdit->clear();
    m_startTimeEdit->setDateTime(QDateTime::currentDateTime().addSecs(-3600));
    m_endTimeEdit->setDateTime(QDateTime::currentDateTime());
    applyFilters();
  });

  row2Layout->addWidget(timeLabel);
  row2Layout->addWidget(m_startTimeEdit);
  row2Layout->addWidget(toLabel);
  row2Layout->addWidget(m_endTimeEdit);
  row2Layout->addSpacing(10);
  row2Layout->addWidget(qcUserLabel);
  row2Layout->addWidget(m_qcUserEdit);
  row2Layout->addSpacing(10);
  row2Layout->addWidget(qcResultLabel);
  row2Layout->addWidget(m_qcResultCombo);
  row2Layout->addSpacing(8);
  row2Layout->addWidget(m_resetButton);
  row2Layout->addStretch(1);
  searchRoot->addWidget(row2);

  QFrame* line = new QFrame(searchPanel);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Plain);
  line->setStyleSheet(QStringLiteral("color:#b9b9b9;"));
  searchRoot->addWidget(line);

  m_listView = new QListView(this);
  m_listView->setViewMode(QListView::IconMode);
  m_listView->setResizeMode(QListView::Adjust);
  m_listView->setFlow(QListView::LeftToRight);
  m_listView->setWrapping(true);
  m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_listView->setUniformItemSizes(false);
  m_listView->setSpacing(14);
  m_listView->setGridSize(QSize(430, 292));
  m_listView->setMovement(QListView::Static);
  m_listView->setMouseTracking(true);
  m_listView->setFrameShape(QFrame::NoFrame);
  m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  m_model = new QStandardItemModel(this);
  m_delegate = new QualityCardDelegate(m_listView);
  m_listView->setModel(m_model);
  m_listView->setItemDelegate(m_delegate);

  connect(m_listView, &QListView::clicked, this,
          [this](const QModelIndex& index) {
            if (!index.isValid() || index.row() < 0 ||
                index.row() >= m_items.size()) {
              return;
            }
            emit historyDetailRequested(m_items.at(index.row()));
          });

  root->addWidget(brandBar);
  root->addWidget(searchPanel);
  root->addWidget(m_listView, 1);
}

QDateTime HistoryMainViewWidget::parseTaskTime(
    const ImageDistributeInfo& info) {
  const QString raw = info.qualityControlTime.trimmed().isEmpty()
                          ? info.checkTime
                          : info.qualityControlTime;
  QDateTime dt =
      QDateTime::fromString(raw, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
  if (!dt.isValid()) {
    dt = QDateTime::fromString(raw, Qt::ISODate);
  }
  return dt;
}

bool HistoryMainViewWidget::hasRenderableImage(
    const ImageDistributeInfo& info) {
  for (const XrayImage& img : info.pbXrayImages) {
    if (!img.mainXrayImageUrl().isEmpty() ||
        !img.assistXrayImageUrl().isEmpty() ||
        !img.fullXrayImageUrl().isEmpty()) {
      return true;
    }
  }
  for (const XrayImage& img : info.xrayImages) {
    if (!img.mainXrayImageUrl().isEmpty() ||
        !img.assistXrayImageUrl().isEmpty() ||
        !img.fullXrayImageUrl().isEmpty()) {
      return true;
    }
  }
  return false;
}

void HistoryMainViewWidget::setHistoryInfos(
    const QList<ImageDistributeInfo>& infos) {
  m_sourceItems = infos;
  applyFilters();
}

void HistoryMainViewWidget::applyFilters() {
  clearHistoryInfos();

  const QString channelKeyword =
      m_channelEdit ? m_channelEdit->text().trimmed() : QString();
  const QString qcUserKeyword =
      m_qcUserEdit ? m_qcUserEdit->text().trimmed() : QString();
  const QDateTime start =
      m_startTimeEdit ? m_startTimeEdit->dateTime() : QDateTime();
  const QDateTime end = m_endTimeEdit ? m_endTimeEdit->dateTime() : QDateTime();
  const QString aiResult =
      m_aiResultCombo ? m_aiResultCombo->currentText() : QStringLiteral("全部");
  const QString qcResult =
      m_qcResultCombo ? m_qcResultCombo->currentText() : QStringLiteral("全部");
  const QString imageType = m_imageTypeCombo ? m_imageTypeCombo->currentText()
                                             : QStringLiteral("全部");

  for (const ImageDistributeInfo& info : m_sourceItems) {
    if (info.id.trimmed().isEmpty() || !hasRenderableImage(info)) {
      continue;
    }
    if (!channelKeyword.isEmpty()) {
      const QString channelText =
          info.channelNoValue + QStringLiteral(" ") + info.channelNo;
      if (!channelText.contains(channelKeyword, Qt::CaseInsensitive)) {
        continue;
      }
    }
    Q_UNUSED(qcUserKeyword)
    const QDateTime taskTime = parseTaskTime(info);
    if (start.isValid() && taskTime.isValid() && taskTime < start) {
      continue;
    }
    if (end.isValid() && taskTime.isValid() && taskTime > end) {
      continue;
    }
    if (imageType == QStringLiteral("原图") && info.pbXrayImages.isEmpty()) {
      continue;
    }
    if (imageType == QStringLiteral("增强图") && info.xrayImages.isEmpty()) {
      continue;
    }

    QueryInfo* cardData = createCardData(info);
    const QString aiText = channelTextOf(cardData->imageResult);
    const QString qcText = qualityTextOf(cardData->qualityResult);
    if (aiResult != QStringLiteral("全部") && aiText != aiResult) {
      delete cardData;
      continue;
    }
    if (qcResult != QStringLiteral("全部") && qcText != qcResult) {
      delete cardData;
      continue;
    }

    QStandardItem* item = new QStandardItem;
    item->setEditable(false);
    item->setSelectable(true);
    item->setData(QVariant::fromValue(cardData), Qt::UserRole + 1);
    m_model->appendRow(item);
    m_items.append(info);
  }
  qCInfo(lcQcView) << "[HistoryMainViewWidget] cards filtered count="
                   << m_model->rowCount();
}

void HistoryMainViewWidget::clearHistoryInfos() {
  for (int i = 0; i < m_model->rowCount(); ++i) {
    QueryInfo* cardData =
        m_model->item(i)->data(Qt::UserRole + 1).value<QueryInfo*>();
    delete cardData;
  }
  m_items.clear();
  m_model->clear();
}
