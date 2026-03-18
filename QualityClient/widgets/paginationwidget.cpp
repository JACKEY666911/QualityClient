#include "PaginationWidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QIntValidator>
#include <QMessageBox>
#include <QDebug>

PaginationWidget::PaginationWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

PaginationWidget::~PaginationWidget() = default;

// ====== 公开API实现 ======
int PaginationWidget::totalCount() const
{
    return m_totalCount;
}

int PaginationWidget::pageSize() const
{
    return m_pageSize;
}

int PaginationWidget::currentPage() const
{
    return m_currentPage;
}

int PaginationWidget::totalPage() const
{
    return m_totalPage;
}

QList<int> PaginationWidget::pageSizeOptions() const
{
    return m_pageSizeOptions;
}

void PaginationWidget::setTotalCount(int count)
{
    if (count < 0) count = 0;
    if (m_totalCount == count) return;

    m_totalCount = count;
    calculateTotalPage();
    updateWidgetState();
    updatePageInfo();
    emit totalCountChanged(count);
}

void PaginationWidget::setPageSize(int size)
{
    if (size <= 0) size = m_pageSizeOptions.first();
    if (m_pageSize == size) return;

    m_pageSize = size;
    // 切换每页条数后重置为第一页
    int oldPage = m_currentPage;
    m_currentPage = 1;
    calculateTotalPage();
    updateWidgetState();
    updatePageInfo();

    emit pageSizeChanged(size);
    if (oldPage != 1) {
        emit pageChanged(m_currentPage, m_pageSize);
    }
}

void PaginationWidget::setCurrentPage(int page)
{
    // 边界校验
    int targetPage = qBound(1, page, m_totalPage);
    if (m_currentPage == targetPage) return;

    m_currentPage = targetPage;
    updateWidgetState();
    updatePageInfo();
    emit currentPageChanged(targetPage);
    emit pageChanged(targetPage, m_pageSize);
}

void PaginationWidget::setPageSizeOptions(const QList<int> &options)
{
    if (options.isEmpty()) return;
    m_pageSizeOptions = options;

    // 更新下拉框选项
    m_pageSizeCombo->clear();
    for (int size : m_pageSizeOptions) {
        m_pageSizeCombo->addItem(QString("%1条/页").arg(size));
    }
    // 选中当前每页条数
    int index = m_pageSizeOptions.indexOf(m_pageSize);
    if (index >= 0) {
        m_pageSizeCombo->setCurrentIndex(index);
    } else {
        // 若当前size不在选项中，选中第一个
        setPageSize(m_pageSizeOptions.first());
    }
}

void PaginationWidget::reset()
{
    setCurrentPage(1);
    setPageSize(m_pageSizeOptions.first());
}

// ====== 内部交互处理 ======
void PaginationWidget::onFirstPageClicked()
{
    setCurrentPage(1);
}

void PaginationWidget::onPrevPageClicked()
{
    setCurrentPage(m_currentPage - 1);
}

void PaginationWidget::onNextPageClicked()
{
    setCurrentPage(m_currentPage + 1);
}

void PaginationWidget::onLastPageClicked()
{
    setCurrentPage(m_totalPage);
}

void PaginationWidget::onPageJumpClicked()
{
    bool ok;
    int page = m_pageEdit->text().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, tr("提示"), tr("请输入有效的页码！"));
        m_pageEdit->clear();
        return;
    }
    setCurrentPage(page);
    m_pageEdit->clear();
}

void PaginationWidget::onPageSizeChanged(int index)
{
    if (index < 0 || index >= m_pageSizeOptions.size()) return;
    setPageSize(m_pageSizeOptions[index]);
}

// ====== UI初始化与状态更新 ======
void PaginationWidget::initUI()
{
    // 主布局：水平布局，间距10px，边距10px
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 5, 10, 5);
    mainLayout->setSpacing(10);

    // 1. 总条数标签
    m_totalLabel = new QLabel(tr("共 0 条数据"), this);
    mainLayout->addWidget(m_totalLabel);

    // 2. 分页按钮组
    m_firstPageBtn = new QPushButton(tr("首页"), this);
    m_prevPageBtn = new QPushButton(tr("上一页"), this);
    m_nextPageBtn = new QPushButton(tr("下一页"), this);
    m_lastPageBtn = new QPushButton(tr("尾页"), this);
    mainLayout->addWidget(m_firstPageBtn);
    mainLayout->addWidget(m_prevPageBtn);
    mainLayout->addWidget(m_nextPageBtn);
    mainLayout->addWidget(m_lastPageBtn);

    // 3. 页码信息
    m_pageInfoLabel = new QLabel(tr("第 1 页 / 共 0 页"), this);
    mainLayout->addWidget(m_pageInfoLabel);

    // 4. 页码跳转
    mainLayout->addWidget(new QLabel(tr("跳转到"), this));
    m_pageEdit = new QLineEdit(this);
    m_pageEdit->setFixedWidth(50);
    m_pageEdit->setValidator(new QIntValidator(1, 99999, this)); // 限制数字输入
    mainLayout->addWidget(m_pageEdit);
    m_jumpBtn = new QPushButton(tr("跳转"), this);
    mainLayout->addWidget(m_jumpBtn);

    // 5. 每页条数（右对齐）
    mainLayout->addStretch();
    mainLayout->addWidget(new QLabel(tr("每页显示"), this));
    m_pageSizeCombo = new QComboBox(this);
    mainLayout->addWidget(m_pageSizeCombo);

    // 绑定信号槽
    connect(m_firstPageBtn, &QPushButton::clicked, this, &PaginationWidget::onFirstPageClicked);
    connect(m_prevPageBtn, &QPushButton::clicked, this, &PaginationWidget::onPrevPageClicked);
    connect(m_nextPageBtn, &QPushButton::clicked, this, &PaginationWidget::onNextPageClicked);
    connect(m_lastPageBtn, &QPushButton::clicked, this, &PaginationWidget::onLastPageClicked);
    connect(m_jumpBtn, &QPushButton::clicked, this, &PaginationWidget::onPageJumpClicked);
    connect(m_pageSizeCombo, &QComboBox::currentIndexChanged, this, &PaginationWidget::onPageSizeChanged);

    // 初始化下拉框
    setPageSizeOptions(m_pageSizeOptions);
    // 初始状态更新
    updateWidgetState();
}

void PaginationWidget::calculateTotalPage()
{
    if (m_totalCount == 0 || m_pageSize == 0) {
        m_totalPage = 0;
        return;
    }
    // 向上取整计算总页数
    m_totalPage = (m_totalCount + m_pageSize - 1) / m_pageSize;
    // 确保当前页不超过总页数
    if (m_currentPage > m_totalPage && m_totalPage > 0) {
        setCurrentPage(m_totalPage);
    }
}

void PaginationWidget::updateWidgetState()
{
    // 无数据时禁用所有交互
    bool hasData = (m_totalCount > 0);
    bool canPrev = hasData && (m_currentPage > 1);
    bool canNext = hasData && (m_currentPage < m_totalPage);

    m_firstPageBtn->setEnabled(canPrev);
    m_prevPageBtn->setEnabled(canPrev);
    m_nextPageBtn->setEnabled(canNext);
    m_lastPageBtn->setEnabled(canNext);
    m_jumpBtn->setEnabled(hasData);
    m_pageEdit->setEnabled(hasData);
    m_pageSizeCombo->setEnabled(hasData);
}

void PaginationWidget::updatePageInfo()
{
    // 更新总条数显示
    m_totalLabel->setText(tr("共 %1 条数据").arg(m_totalCount));
    // 更新页码信息
    if (m_totalPage == 0) {
        m_pageInfoLabel->setText(tr("第 0 页 / 共 0 页"));
    } else {
        m_pageInfoLabel->setText(tr("第 %1 页 / 共 %2 页").arg(m_currentPage).arg(m_totalPage));
    }
}