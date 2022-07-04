#include "container.h"

#include <QIcon>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>

#include <iostream>

ContainerListView::ContainerListView(QWidget *p) : AdvancedListView(p)
{
	setFlow(QListView::Flow::LeftToRight);
	setWrapping(true);
	setResizeMode(QListView::ResizeMode::Adjust);
	setSpacing(0);
	setFrameStyle(QFrame::Plain);
	setLineWidth(0);

	ContainerDelegate *del = new ContainerDelegate(this);
	setAdvancedModel(new ContainerModel(this));
	setAdvancedDelegate(del);

	setRemovable(true);
	setHovers(true);

	connect(del, &ContainerDelegate::clicked, this, [this](QString s){std::cout << "Clicked on: " << s.toStdString() << std::endl; Q_EMIT selectionChanged(s);});
}

ContainerModel *ContainerListView::getContainerModel()
{
	return (ContainerModel *)model();
}

ContainerDelegate::ContainerDelegate(AdvancedListView *p) : AdvancedItemDelegate(p)
{
}

void ContainerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing);
	painter->setRenderHint(QPainter::SmoothPixmapTransform);

	QString text = index.data(ContainerModel::Accesses::Text).toString();
	QPixmap ico = index.data(ContainerModel::Accesses::Pixmap).value<QPixmap>();

	QSize s = sizeHint(opt, index);
	QPoint p = opt.rect.topLeft();
	QTextOption topt;
	p += QPoint(2, 2);
	s -= QSize(2, 4);
	topt.setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	//Put the box.
	QPainterPath box;
	box.addRoundedRect(QRect(QPoint(0, 0) + p, s), 8, 8);
 	painter->setPen(QPen(opt.palette.color(QPalette::ColorGroup::Current, QPalette::ColorRole::Mid), 1));
	painter->drawPath(box);
	painter->setClipPath(box);
	painter->fillPath(box, (opt.state & QStyle::State_MouseOver
	? opt.palette.color(QPalette::ColorGroup::Current, QPalette::ColorRole::Button)
	: opt.palette.color(QPalette::ColorGroup::Current, QPalette::ColorRole::Base)));

	painter->setPen(opt.state & QStyle::State_MouseOver
	? opt.palette.color(QPalette::ColorGroup::Current, QPalette::ColorRole::ButtonText)
	: opt.palette.color(QPalette::ColorGroup::Current, QPalette::ColorRole::Text));

	p += QPoint(1, 1);
	//Put the icon
	if (ico.isNull()) {
		painter->fillRect(QRect(p, QSize(24, 24)), Qt::red);
	} else {
		painter->drawPixmap(QRect(p, QSize(24, 24)), ico.scaled(24, 24), QRect(0, 0, 24, 24));
	}

	//Put the text
	if (m_enableRemove && opt.state & QStyle::State_MouseOver) {
		s -= QSize(30, 0);
	}
	painter->drawText(QRect(p + QPoint(30, 0), s), text, topt);

	if (m_enableRemove) {
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

QSize ContainerDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
	return QSize(m_parent->size().width() - 10, 30);
}

QVariant ContainerDelegate::isInDecoration(const QPoint &p, const QModelIndex &index, const QStyleOptionViewItem &opt) const
{
	return QRect(3 + sizeHint(opt, index).width() - 25, 5, 20, 20).contains(p);
}

QVariant ContainerDelegate::handleRelease(const QPoint &p, const QModelIndex &index, const QStyleOptionViewItem &opt)
{
	if (m_selectedRow != -1) {
		Q_EMIT clicked(index.data(ContainerModel::Accesses::Text).toString());
	}
	return AdvancedItemDelegate::handleRelease(p, index, opt);
}

ContainerModel::ContainerModel(QObject *p) : AdvancedItemModel(p), wrap(new SWrapper(this))
{
}

void ContainerModel::setWrapper(SWrapper *w)
{
	clear();
	if (w) {
		QList<QString> tmp = w->containers();
		beginInsertRows(createIndex(0, 0), 0, tmp.length());
		containers = tmp;
		endInsertRows();
		Q_EMIT dataChanged(createIndex(0, 0), createIndex(containers.count(), 0));
		Q_EMIT dataUpdated();
	}
	wrap = w;
}

int ContainerModel::rowCount(const QModelIndex &) const
{
	return containers.count();
}

int ContainerModel::columnCount(const QModelIndex &) const
{
	return 1;
}

QModelIndex ContainerModel::parent(const QModelIndex &) const
{
	return QModelIndex();
}

QVariant ContainerModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < containers.count() && index.row() > -1 && wrap) {
		switch(role) {
			case Accesses::Text: {
				return containers[index.row()];
			}
			case Accesses::Pixmap: {
				return QVariant::fromValue(wrap->getPixmap(containers[index.row()]));
			}
		}
	}
	return QVariant();
}

QModelIndex ContainerModel::index(int row, int column, const QModelIndex &parent) const
{
	if (hasIndex(row, column, parent)) {
		return createIndex(row, column);
	}
	return QModelIndex();
}

void ContainerModel::removeData(int r)
{
	beginRemoveRows(createIndex(r, 0), r, r);
	containers.removeAt(r);
	endRemoveRows();
	Q_EMIT dataChanged(createIndex(r, 0), createIndex(r, 0));
	Q_EMIT dataUpdated();
}

void ContainerModel::clear()
{
	int c = containers.count()-1;
	beginRemoveRows(createIndex(0, 0), 0, c);
	containers.clear();
	endRemoveRows();
	Q_EMIT dataChanged(createIndex(0, 0), createIndex(c, 0));
}

void ContainerModel::handlePress(QVariant)
{}

void ContainerModel::handleRelease(QVariant v)
{
	int r = v.toInt();
	if (hasIndex(r, 0)) {
		removeData(r);
		Q_EMIT dataUpdated();
	}
}

