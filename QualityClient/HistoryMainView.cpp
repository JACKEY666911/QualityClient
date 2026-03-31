#include "HistoryMainView.h"

#include <QTimer>

#include "ui_HistoryMainView.h"

HistoryMainView::HistoryMainView(QWidget *parent)
    : QWidget(parent), ui(new Ui::HistoryMainView) {
  ui->setupUi(this);
  m_model = new QStandardItemModel(this);
  m_delegate = new QualityCardDelegate(this);
  m_pagination = new PaginationWidget(this);
  m_listView = new QListView(this);

  setupListView();

  // 1. 设置每页可选的条数
  m_pagination->setPageSizeOptions({12, 24, 48, 96});
  QHBoxLayout *hLayout = new QHBoxLayout();
  hLayout->addSpacing(50);
  hLayout->addWidget(m_pagination);
  hLayout->addSpacing(50);
  // 2. 连接信号：当用户点击翻页或切换每页条数时，触发重新加载数据
  connect(m_pagination, &PaginationWidget::pageChanged, this,
          &HistoryMainView::loadData);
  ui->verticalLayoutContent->addItem(hLayout);
  ui->verticalLayoutContent->addWidget(m_listView, 1);
  loadData(1, 12);
}

HistoryMainView::~HistoryMainView() { delete ui; }

void HistoryMainView::loadData(int page, int pageSize) {
  // 防抖锁，防止 Qt6 异步场景下的并发重复请求
  if (m_isLoading)
    return;

  m_model->clear();

  // 模拟业务：假设接口返回了最新总数
  int totalCount = 67;

  // C++17: 使用 std::clamp 和 constexpr 风格计算总页数
  int realTotalPage =
      (totalCount > 0) ? (totalCount + pageSize - 1) / pageSize : 0;

  // 边界防御：如果请求页码越界，让 PaginationWidget 内部修正，不发无意义请求
  if (page > realTotalPage && realTotalPage > 0) {
    m_pagination->setTotalCount(totalCount);
    return;
  }

  m_isLoading = true;
  m_pagination->setTotalCount(totalCount);

  // 模拟网络异步请求
  QTimer::singleShot(300, this, [this, page, pageSize, totalCount]() {
    int startIndex = (page - 1) * pageSize;
    int endIndex = std::min(startIndex + pageSize, totalCount);

    static const QStringList lines = {"LJ01", "LJ05", "LJ09", "LJ12"};
    static const QStringList types = {"首检", "巡检", "抽检", "复检"};

    for (int i = startIndex; i < endIndex; ++i) {
      auto *item = new QStandardItem();

      item->setData(QString("2026-03-26 %1:%2:%3")
                        .arg(8 + (i % 12), 2, 10, QLatin1Char('0'))
                        .arg(i % 60, 2, 10, QLatin1Char('0'))
                        .arg((i * 7) % 60, 2, 10, QLatin1Char('0')),
                    Qt::UserRole + 1);

      item->setData(QString("%1 | %2").arg(lines[i % 4], types[i % 4]),
                    Qt::UserRole + 2);

      item->setData(QString(":/Images/yisuoTest1.png"), Qt::UserRole + 3);

      item->setData(i % 5 == 0 ? 0 : 1, Qt::UserRole + 4);
      item->setData(i % 7 == 0 ? 0 : 1, Qt::UserRole + 5);
      item->setData(QString(":/Images/yisuoTest2.png"), Qt::UserRole + 6);

      item->setText("");
      m_model->appendRow(item);
    }

    m_isLoading = false; // 释放锁
  });
}

void HistoryMainView::setupListView() {
  m_listView->setModel(m_model);
  m_listView->setItemDelegate(m_delegate);
  m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_listView->setViewMode(QListView::IconMode);
  m_listView->setFlow(QListView::LeftToRight);
  m_listView->setWrapping(true);

  m_listView->setSpacing(0);
  m_listView->setGridSize(QSize(476, 300));

  m_listView->setResizeMode(QListView::Adjust);

  m_listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_listView->setSelectionMode(QAbstractItemView::NoSelection);
  m_listView->setFrameShape(QFrame::NoFrame);

  m_listView->setStyleSheet(
      "QListView { background: transparent; border: none; outline: none; }");
  m_listView->viewport()->setAutoFillBackground(false);
}
