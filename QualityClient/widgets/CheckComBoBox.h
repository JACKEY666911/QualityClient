#ifndef CHECKCOMBOBOX_H
#define CHECKCOMBOBOX_H

#include <QComboBox>
#include <QPointer>
#include <QWidget>
#include <QAbstractListModel>
struct AreaInfo
{
    int id;
    QString areaName;
    Qt::CheckState checkState;
    AreaInfo():id(-1), checkState(Qt::Unchecked){}
    AreaInfo(int id, const QString &name, Qt::CheckState state = Qt::Unchecked):id(id), areaName(name), checkState(state){}
    // 辅助函数
    bool isValid() const { return id >= 0 && !areaName.isEmpty(); }

    // 切换选中状态
    void toggle() {
        if (checkState == Qt::Checked)
            checkState = Qt::Unchecked;
        else
            checkState = Qt::Checked;
    }
};
// 注册为元类型，以便在QVariant中使用
Q_DECLARE_METATYPE(AreaInfo)


class AreaInfoModel: public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles
    {
        IdRole = Qt::UserRole + 1,
        AreaNameRole,
        CheckStateRole,
        AreaInfoRole
    };

    Q_ENUM(Roles)

    explicit AreaInfoModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

public:
    void setAreas(const QList<AreaInfo>& areas);
    QList<AreaInfo> checkedAreas() const;

private:
    QList<AreaInfo> m_areas;
};

class CheckComBoBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CheckComBoBox(QWidget *parent = nullptr);

    void setAreas(const QList<AreaInfo> &areas);
    QList<AreaInfo> checkedAreas() const;
    QList<int> checkedIds() const;

protected:
    void showPopup() override;
    void hidePopup() override;

private slots:
    void updateText();


signals:

private:
    bool m_blockHide = false;
};

#endif // CHECKCOMBOBOX_H
