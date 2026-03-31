#include "CheckComBoBox.h"
#include "CheckBoxDelegate.h"
#include <QEvent>
#include <QLineEdit>
#include <QListView>
#include <QMouseEvent>
#include <QScrollBar>

CheckComBoBox::CheckComBoBox(QWidget *parent) : QComboBox(parent) {
  // 1. 设置可编辑但不可手动输入
  setEditable(true);
  lineEdit()->setReadOnly(true);

  // 【关键】禁止文本交互，防止产生光标、蓝底选区
  lineEdit()->setFocusPolicy(Qt::NoFocus);
  lineEdit()->setCursor(Qt::PointingHandCursor);

  // 2. QLineEdit 样式设置
  lineEdit()->setStyleSheet(R"(
        QLineEdit {
            background-color:#F5F7FA;
            border:none;
            border-radius:8px;
            padding-left:8px;
            color:#333;
            font-size:14px;
        }
    )");

  // 3. QComboBox 整体样式
  this->setStyleSheet(R"(
        QComboBox {
            font-size:14px;
            min-height:24px;
            border: 1px solid #DCDFE6;
            border-radius: 4px;
            padding: 3px 25px 3px 8px;
            background-color: white;
            color: #333333;
            font-family: "Microsoft YaHei";
        }
        QComboBox::drop-down {
            border:none;
            width:30px;
        }
        QComboBox::down-arrow {
            image: url(:/Images/arrow-Down.png);
            width: 15px; height: 15px;
        }
        QComboBox::down-arrow:on {
            image: url(:/Images/arrow-On.png);
            width: 15px; height: 15px;
        }
    )");

  // 4. 配置下拉视图 QListView
  // QListView *view = new QListView(this);
  this->view()->setSelectionMode(QAbstractItemView::NoSelection);
  this->view()->setStyleSheet(R"(
        QListView {
            background-color:#FFFFFF;
            border:1px solid #E4E7ED;
            border-radius:6px;
            padding:4px;
            outline: 0px;
        }
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #DCDFE6;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background: #C0C4CC;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");

  // 配置视图行为
  this->view()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  this->view()->verticalScrollBar()->setSingleStep(10);
  this->view()->setEditTriggers(QAbstractItemView::NoEditTriggers);
  this->view()->setMouseTracking(true);               // 关键！开启鼠标跟踪
  this->view()->viewport()->installEventFilter(this); // 双重保险
  // 5. 设置 Model 和 Delegate
  setModel(new AreaInfoModel(this));
  setItemDelegate(new CheckBoxDelegate(this));

  this->setMaxVisibleItems(5);

  // 6. 信号槽
  connect(model(), &QAbstractItemModel::dataChanged, this,
          &CheckComBoBox::updateText);

  updateText();
}

void CheckComBoBox::setAreas(const QList<AreaInfo> &areas) {
  static_cast<AreaInfoModel *>(model())->setAreas(areas);
  updateText();
}

QList<AreaInfo> CheckComBoBox::checkedAreas() const {
  return static_cast<AreaInfoModel *>(model())->checkedAreas();
}

QList<int> CheckComBoBox::checkedIds() const {
  QList<int> ids;
  for (const auto &a : checkedAreas()) {
    ids << a.id;
  }
  return ids;
}

// 保持 Popup 状态管理（与 eventFilter 形成双保险）
void CheckComBoBox::showPopup() { QComboBox::showPopup(); }

void CheckComBoBox::hidePopup() { QComboBox::hidePopup(); }

bool CheckComBoBox::eventFilter(QObject *watched, QEvent *event) {
  // 关键修改：去掉 isVisible()，同时检查 view 和 viewport
  if (watched != view() && watched != view()->viewport()) {
    return QComboBox::eventFilter(watched, event);
  }

  if (event->type() == QEvent::MouseButtonRelease) {

    QMouseEvent *me = static_cast<QMouseEvent *>(event);

    // 使用 viewport 映射坐标
    QPoint pos =
        view()->viewport()->mapFromGlobal(me->globalPosition().toPoint());
    QModelIndex index = view()->indexAt(pos);

    qDebug() << "Mouse event at:" << pos << "index valid:" << index.isValid();

    if (index.isValid()) {
      Qt::CheckState state = static_cast<Qt::CheckState>(
          model()->data(index, Qt::CheckStateRole).toInt());
      model()->setData(index,
                       state == Qt::Checked ? Qt::Unchecked : Qt::Checked,
                       Qt::CheckStateRole);
      return true;
    }
  }

  return QComboBox::eventFilter(watched, event);
}

void CheckComBoBox::updateText() {
  auto m = static_cast<AreaInfoModel *>(model());
  auto list = m->checkedAreas();

  if (list.isEmpty()) {
    lineEdit()->clear();
    return;
  }

  QStringList names;
  for (const auto &a : list) {
    names << a.areaName;
  }

  lineEdit()->setText(names.join(", "));
  lineEdit()->deselect();
}

AreaInfoModel::AreaInfoModel(QObject *parent) : QAbstractListModel(parent) {}

int AreaInfoModel::rowCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : m_areas.size();
}

QVariant AreaInfoModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= m_areas.size())
    return QVariant();

  const AreaInfo &area = m_areas.at(index.row());
  if (role == Qt::DisplayRole || role == AreaNameRole)
    return area.areaName;
  if (role == Qt::CheckStateRole || role == CheckStateRole)
    return area.checkState;
  if (role == IdRole)
    return area.id;
  if (role == AreaInfoRole)
    return QVariant::fromValue(area);

  return QVariant();
}

bool AreaInfoModel::setData(const QModelIndex &index, const QVariant &value,
                            int role) {
  if (!index.isValid() || index.row() >= m_areas.size())
    return false;

  if (role == Qt::CheckStateRole) {
    m_areas[index.row()].checkState =
        static_cast<Qt::CheckState>(value.toInt());
    emit dataChanged(index, index, {Qt::CheckStateRole});
    return true;
  }
  return false;
}

Qt::ItemFlags AreaInfoModel::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return Qt::NoItemFlags;
  return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void AreaInfoModel::setAreas(const QList<AreaInfo> &areas) {
  beginResetModel();
  m_areas = areas;
  endResetModel();
}

QList<AreaInfo> AreaInfoModel::checkedAreas() const {
  QList<AreaInfo> list;
  for (const auto &a : m_areas)
    if (a.checkState == Qt::Checked)
      list << a;
  return list;
}