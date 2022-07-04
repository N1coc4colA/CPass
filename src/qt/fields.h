#pragma once

#include <string>
#include <list>
#include <vector>

#include "advancedclasses.h"
#include "swrapper.h"

class FieldsViewport : public QWidget
{
	Q_OBJECT
public:
	explicit FieldsViewport(QWidget *p = nullptr);

protected:
	void paintEvent(QPaintEvent *e);
};

class FieldsModel : public AdvancedItemModel
{
	Q_OBJECT
public:
	explicit FieldsModel(QObject *p = nullptr);
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
		Content
	};

	void setTargetContainer(QString n);

private:
	QList<QString> fields;
	QString fieldName = "";
	SWrapper *wrap = nullptr;

	void setupContent();
};

class FieldsDelegate : public AdvancedItemDelegate
{
	Q_OBJECT
public:
	explicit FieldsDelegate(AdvancedListView *p = nullptr);

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;

	bool isInIcon(const QPoint &p, const QModelIndex &index, const QStyleOptionViewItem &opt) const;

protected:
	QVariant handleRelease(const QPoint &p, const QModelIndex &index, const QStyleOptionViewItem &opt) override;
	QVariant handlePress(const QPoint &p, const QModelIndex &index, const QStyleOptionViewItem &opt) override;
	QVariant isInDecoration(const QPoint &p, const QModelIndex &index, const QStyleOptionViewItem &opt) const override;

	bool m_isInIcon = false;
};

class FieldsListView : public AdvancedListView
{
	Q_OBJECT
public:
	explicit FieldsListView(QWidget *p = nullptr);

	void setContainer(QString n);
	FieldsModel *getFieldsModel();
};
