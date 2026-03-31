#ifndef HISTORYMAINVIEW_H
#define HISTORYMAINVIEW_H

#include <widgets/PaginationWidget.h>
#include <widgets/QualityCardDelegate.h>

#include <QListView>
#include <QStandardItemModel>
#include <QWidget>

namespace Ui {
class HistoryMainView;
}

class HistoryMainView : public QWidget {
  Q_OBJECT

public:
  explicit HistoryMainView(QWidget *parent = nullptr);
  ~HistoryMainView();
private slots:
  void loadData(int page, int pageSize);
  void setupListView();

private:
  Ui::HistoryMainView *ui;

  PaginationWidget *m_pagination;
  QListView *m_listView;
  QStandardItemModel *m_model;
  QualityCardDelegate *m_delegate;
  bool m_isLoading = false;
};

#endif // HISTORYMAINVIEW_H
