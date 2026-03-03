#include "CheckComBoBox.h"

#include <QListView>
#include <QLineEdit>
CheckComBoBox::CheckComBoBox(QWidget *parent) : QComboBox(parent)
{
    setModel(new AreaInfoModel(this));
//    QListView *listView = new QListView(this);
//    listView->setItemDelegate(new CheckBoxDelegate(listView));
//    setView(listView);
//    setView(new QListView(this));
    setItemDelegate(new CheckBoxDelegate(this));

    setEditable(true);
    lineEdit()->setReadOnly(true);
    connect(model(), &QAbstractItemModel::dataChanged, this, &CheckComBoBox::updateText);
    updateText();
}

void CheckComBoBox::setAreas(const QList<AreaInfo> &areas)
{
    static_cast<AreaInfoModel*>(model())->setAreas(areas);
    updateText();
}

QList<AreaInfo> CheckComBoBox::checkedAreas() const
{
    return static_cast<AreaInfoModel*>(model())->checkedAreas();
}

QList<int> CheckComBoBox::checkedIds() const
{
    QList<int> ids;
    for (const auto &a : checkedAreas()) {
        ids << a.id;
    }
    return ids;
}

void CheckComBoBox::showPopup()
{
    m_blockHide = true;
    QComboBox::showPopup();
}

void CheckComBoBox::hidePopup()
{
    if(!m_blockHide)
    {
        QComboBox::hidePopup();
    }
    else
    {
        m_blockHide = false;
    }
}

void CheckComBoBox::updateText()
{
    auto m = static_cast<AreaInfoModel*>(model());
    auto list = m->checkedAreas();

    if (list.isEmpty()) {
        lineEdit()->clear();
        return;
    }

    QStringList names;
    for (const auto& a : list)
        names << a.areaName;

    lineEdit()->setText(names.join(", "));
}


AreaInfoModel::AreaInfoModel(QObject *parent)
{

}

int AreaInfoModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_areas.size();
}

QVariant AreaInfoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_areas.size())
        return QVariant();

    const AreaInfo &area = m_areas.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case AreaNameRole:
        return area.areaName;
    case Qt::CheckStateRole:
    case CheckStateRole:
        return area.checkState;
    case IdRole:
        return area.id;
    case AreaInfoRole:
           return QVariant::fromValue(area);
    default:
        return {};
    };
}

bool AreaInfoModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid())
    {
        return false;
    }
    AreaInfo &area = m_areas[(index.row())];
    if(role == Qt::CheckStateRole)
    {
        area.checkState = static_cast<Qt::CheckState>(value.toInt());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags AreaInfoModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

void AreaInfoModel::setAreas(const QList<AreaInfo> &areas)
{
    beginResetModel();
    m_areas = areas;
    endResetModel();
}

QList<AreaInfo> AreaInfoModel::checkedAreas() const
{
    QList<AreaInfo> list;
    for (const auto& a : m_areas)
        if (a.checkState == Qt::Checked)
            list << a;
    return list;
}
