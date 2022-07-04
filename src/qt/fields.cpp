#include "fields.h"

#include <QIcon>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPaintEvent>

#include <iostream>

FieldsViewport::FieldsViewport(QWidget *p) : QWidget(p)
{
}

void FieldsViewport::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	p.fillRect(e->rect(), palette().color(QPalette::ColorGroup::Current, QPalette::ColorRole::Dark));
}

FieldsListView::FieldsListView(QWidget *p) : AdvancedListView(p)
{
	setFlow(QListView::Flow::LeftToRight);
	setWrapping(true);
	setResizeMode(QListView::ResizeMode::Adjust);
	setSpacing(0);
	setFrameStyle(QFrame::Plain);
	setLineWidth(0);

	setAdvancedModel(new FieldsModel(this));
	setAdvancedDelegate(new FieldsDelegate(this));
	setViewport(new FieldsViewport(this));

	setRemovable(true);
	setHovers(true);
}

FieldsModel *FieldsListView::getFieldsModel()
{
	return (FieldsModel *)model();
}

void FieldsListView::setContainer(QString s)
{
	getFieldsModel()->setTargetContainer(s);
}

FieldsDelegate::FieldsDelegate(AdvancedListView *p) : AdvancedItemDelegate(p)
{
}

void FieldsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing);
	painter->setRenderHint(QPainter::SmoothPixmapTransform);

	QString title = index.data(FieldsModel::Accesses::Text).toString();
	QString content = index.data(FieldsModel::Accesses::Content).toString();

	QSize s = sizeHint(opt, index);
	QPoint p = opt.rect.topLeft();
	QTextOption topt;
	p += QPoint(5, 5);
	topt.setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	topt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

	//Put the box.
	QPainterPath box;
	box.addRoundedRect(QRect(QPoint(0, 0) + p, s), 8, 8);
	painter->setClipPath(box);
	painter->fillPath(box, (opt.state & QStyle::State_MouseOver
	? opt.palette.color(QPalette::ColorGroup::Current, QPalette::ColorRole::Button)
	: opt.palette.color(QPalette::ColorGroup::Current, QPalette::ColorRole::Base)));

	painter->setPen(opt.state & QStyle::State_MouseOver
	? opt.palette.color(QPalette::ColorGroup::Current, QPalette::ColorRole::ButtonText)
	: opt.palette.color(QPalette::ColorGroup::Current, QPalette::ColorRole::Text));

	p += QPoint(5, 5);
	//Put the title
	QFont b = opt.font;
	b.setBold(true);
	QFontMetrics mb(b);
	painter->drawText(QRect(p + QPoint(5, 5), QSize(s.width() - 10, 5 + mb.height() + (m_enableRemove ? -25 : 0))), title, topt);

	//Put the content
	painter->drawText(QRect(p + QPoint(5, 5 + mb.height()), QSize(s)), content, topt);

	if (m_enableEdit) {
		//Put the edit button
		int theight = 20;
		QPainterPath box;
		QRect decoRect(QPoint(s.width() - 25, s.height() - 25), QSize(20, 20));
		box.addRoundedRect(decoRect, decoRect.width()/2, decoRect.height()/2);

		QColor c = opt.palette.color(QPalette::ColorGroup::Current, QPalette::ColorRole::Base);
		if (index.row() == m_currentRow && m_isInIcon) {
			c = opt.palette.color(QPalette::ColorGroup::Current, QPalette::ColorRole::Highlight);
		}
		painter->fillPath(box, c);

		//Put the stylo
		QPen pen(opt.palette.color(QPalette::ColorGroup::Current, QPalette::ColorRole::ButtonText), 1);
		pen.setCapStyle(Qt::PenCapStyle::RoundCap);
		painter->setPen(pen);

		const float r = float(theight)/14;
		const QPoint t = decoRect.topLeft();

		box.clear();
		box.addPolygon(QPolygon({QPoint(12, 2)*r+t, QPoint(14, 4)*r+t, QPoint(13, 5)*r+t, QPoint(11, 3)*r+t, QPoint(12, 2)*r+t}));
		box.addPolygon(QPolygon({QPoint(10, 4)*r+t, QPoint(12, 6)*r+t, QPoint(5, 13)*r+t, QPoint(3, 11)*r+t, QPoint(10, 4)*r+t}));
		box.addPolygon(QPolygon({QPoint(2, 12)*r+t, QPoint(2, 14)*r+t, QPoint(4, 14)*r+t, QPoint(2, 12)*r+t}));
		painter->fillPath(box, opt.palette.color(QPalette::ColorGroup::Current, QPalette::ColorRole::ButtonText));
	}

	if (m_enableRemove) {
		//Put the remove button.
		QRect r(p.x() + s.width() - 25, p.y() + 2, 20, 20);

		QPoint n = r.topLeft();
		QPainterPath path;

		if (index.row() == m_currentRow) {
			path.addRoundedRect(r, 5, 5);
		} else {
			path.addEllipse(r);
		}
		painter->fillPath(path, Qt::red);

		QPen lpen(Qt::white, 2);
		lpen.setCapStyle(Qt::PenCapStyle::RoundCap);
		painter->setPen(lpen);
		painter->drawLine(QPoint(5, 10) +n, QPoint(15, 10) +n);
	}

	painter->restore();
}

QSize FieldsDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	QFont b = opt.font;
	b.setBold(true);
	QFontMetrics mb(b);

	QSize s = opt.fontMetrics.boundingRect(0, 0, m_parent->width() - 10, INT_MAX, (Qt::AlignVCenter | Qt::AlignLeft), index.data(FieldsModel::Accesses::Content).toString()).size();

	return QSize(m_parent->width() - 10, s.height() + 35 + mb.height());
}

bool FieldsDelegate::isInIcon(const QPoint &p, const QModelIndex &index, const QStyleOptionViewItem &opt) const
{
	QSize s = sizeHint(opt, index);
	return QRect(s.width() - 25, s.height() - 25, 20, 20).contains(p);
}

QVariant FieldsDelegate::isInDecoration(const QPoint &p, const QModelIndex &index, const QStyleOptionViewItem &opt) const
{
	if (m_enableEdit && isInIcon(p, index, opt)) {
		return 2;
	}
	return QRect(3 + sizeHint(opt, index).width() - 25, 5, 20, 20).contains(p);
}

QVariant FieldsDelegate::handleRelease(const QPoint &p, const QModelIndex &index, const QStyleOptionViewItem &opt)
{
	QVariant v = AdvancedItemDelegate::handleRelease(p, index, opt);
	m_isInIcon = false;
	return v;
}

QVariant FieldsDelegate::handlePress(const QPoint &p, const QModelIndex &index, const QStyleOptionViewItem &opt)
{
	m_currentRow = index.row();
	m_isInIcon = isInIcon(p, index, opt);
	return QVariant();
}

FieldsModel::FieldsModel(QObject *p) : AdvancedItemModel(p), wrap(new SWrapper(this))
{
}

void FieldsModel::setWrapper(SWrapper *w)
{
	clear();
	setupContent();
	wrap = w;
}

int FieldsModel::rowCount(const QModelIndex &) const
{
	return fields.count();
}

int FieldsModel::columnCount(const QModelIndex &) const
{
	return 1;
}

QModelIndex FieldsModel::parent(const QModelIndex &) const
{
	return QModelIndex();
}

QVariant FieldsModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < fields.count() && index.row() > -1 && wrap) {
		switch(role) {
			case Accesses::Text: {
				return fields[index.row()];
			}
			case Accesses::Content: {
				return wrap->value(fieldName, fields[index.row()]);
			}
		}
	}
	return QVariant();
}

QModelIndex FieldsModel::index(int row, int column, const QModelIndex &parent) const
{
	if (hasIndex(row, column, parent)) {
		return createIndex(row, column);
	}
	return QModelIndex();
}

void FieldsModel::removeData(int r)
{
	beginRemoveRows(createIndex(r, 0), r, r);
	fields.removeAt(r);
	endRemoveRows();
	Q_EMIT dataChanged(createIndex(r, 0), createIndex(r, 0));
	Q_EMIT dataUpdated();
}

void FieldsModel::clear()
{
	int c = fields.count()-1;
	beginRemoveRows(createIndex(0, 0), 0, c);
	fields.clear();
	endRemoveRows();
	Q_EMIT dataChanged(createIndex(0, 0), createIndex(c, 0));
}

void FieldsModel::handlePress(QVariant)
{
}

void FieldsModel::handleRelease(QVariant v)
{
	int r = v.toInt();
	if (hasIndex(r, 0)) {
		removeData(r);
		Q_EMIT dataUpdated();
	}
}

void FieldsModel::setTargetContainer(QString n)
{
	clear();
	fieldName = n;
	setupContent();
}

void FieldsModel::setupContent()
{
	if (wrap) {
		QList<QString> tmp = wrap->fields(fieldName);
		std::cout << "Length: " << tmp.length() << std::endl;
		beginInsertRows(createIndex(0, 0), 0, tmp.length());
		fields = tmp;
		endInsertRows();
		Q_EMIT dataChanged(createIndex(0, 0), createIndex(fields.count(), 0));
		Q_EMIT dataUpdated();
	}
}
