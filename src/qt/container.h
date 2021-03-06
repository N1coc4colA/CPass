#pragma once

#include <string>
#include <list>
#include <vector>

#include "advancedclasses.h"
#include "swrapper.h"

class ContainerModel : public AdvancedItemModel
{
	Q_OBJECT
public:
	explicit ContainerModel(QObject *p = nullptr);
	void setWrapper(SWrapper *w);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &) const override;

	void clear() override;
	void handlePress(QVariant v) override;
	void handleRelease(QVariant v) override;

	void removeData(int row);

	enum Accesses {
		Text = Qt::DisplayRole +1,
		Pixmap
	};

private:
	QList<QString> containers;
	SWrapper *wrap = nullptr;
};

class ContainerDelegate : public AdvancedItemDelegate
{
	Q_OBJECT
public:
	explicit ContainerDelegate(AdvancedListView *p = nullptr);

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;

Q_SIGNALS:
	void clicked(QString);

protected:
	QVariant isInDecoration(const QPoint &p, const QModelIndex &index, const QStyleOptionViewItem &opt) const override;
	QVariant handleRelease(const QPoint &p, const QModelIndex &index, const QStyleOptionViewItem &opt) override;
};

class ContainerListView : public AdvancedListView
{
	Q_OBJECT
public:
	explicit ContainerListView(QWidget *p = nullptr);

	ContainerModel *getContainerModel();

Q_SIGNALS:
	void selectionChanged(QString);
};
