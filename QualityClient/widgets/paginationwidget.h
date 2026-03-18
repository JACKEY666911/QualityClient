#ifndef PAGINATIONWIDGET_H
#define PAGINATIONWIDGET_H

#include <QWidget>
#include <QList>
#include <QSize>

class QLabel;
class QLineEdit;
class QPushButton;
class QComboBox;

/**
 * @brief 工业级独立分页控件（无业务依赖）
 * 特性：
 * 1. 完全解耦，仅传递分页参数
 * 2. 支持自定义每页条数选项
 * 3. 自动处理边界状态（禁用无效按钮）
 * 4. 支持QSS样式定制
 */
class PaginationWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int totalCount READ totalCount WRITE setTotalCount NOTIFY totalCountChanged)
    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(QList<int> pageSizeOptions READ pageSizeOptions WRITE setPageSizeOptions)

public:
    explicit PaginationWidget(QWidget *parent = nullptr);
    ~PaginationWidget() override;

    // 分页参数设置/获取
    int totalCount() const;          // 总条数
    int pageSize() const;            // 每页条数
    int currentPage() const;         // 当前页
    int totalPage() const;           // 总页数（自动计算）
    QList<int> pageSizeOptions() const; // 每页条数可选值

    void setTotalCount(int count);   // 设置总条数
    void setPageSize(int size);      // 设置每页条数
    void setCurrentPage(int page);   // 设置当前页（自动校验边界）
    void setPageSizeOptions(const QList<int> &options); // 设置每页条数选项

    // 重置分页（恢复到第一页）
    void reset();

signals:
    // 分页变更信号（核心：向外传递分页参数）
    void pageChanged(int currentPage, int pageSize);
    // 属性变更信号（供UI绑定）
    void totalCountChanged(int count);
    void pageSizeChanged(int size);
    void currentPageChanged(int page);

private slots:
    // 内部交互处理
    void onFirstPageClicked();
    void onPrevPageClicked();
    void onNextPageClicked();
    void onLastPageClicked();
    void onPageJumpClicked();
    void onPageSizeChanged(int index);

private:
    // 初始化UI
    void initUI();
    // 计算总页数
    void calculateTotalPage();
    // 更新控件状态（禁用/启用按钮）
    void updateWidgetState();
    // 更新页码信息显示
    void updatePageInfo();

private:
    // 分页参数
    int m_totalCount = 0;            // 总条数
    int m_pageSize = 10;             // 每页条数
    int m_currentPage = 1;           // 当前页
    int m_totalPage = 0;             // 总页数
    QList<int> m_pageSizeOptions = {10, 20, 50, 100}; // 默认每页条数选项

    // UI控件（可通过QSS定制）
    QLabel *m_totalLabel = nullptr;        // 总条数标签
    QLabel *m_pageInfoLabel = nullptr;     // 页码信息标签（第X页/共Y页）
    QLineEdit *m_pageEdit = nullptr;       // 页码输入框
    QPushButton *m_jumpBtn = nullptr;      // 跳转按钮
    QComboBox *m_pageSizeCombo = nullptr;  // 每页条数下拉框
    QPushButton *m_firstPageBtn = nullptr; // 首页按钮
    QPushButton *m_prevPageBtn = nullptr;  // 上一页按钮
    QPushButton *m_nextPageBtn = nullptr;  // 下一页按钮
    QPushButton *m_lastPageBtn = nullptr;  // 尾页按钮
};

#endif // PAGINATIONWIDGET_H