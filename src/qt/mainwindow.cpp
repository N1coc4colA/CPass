#include "mainwindow.h"

#include <QVBoxLayout>
#include <QSplitter>

#include "windowattacheddata.h"
#include "container.h"
#include "fields.h"
#include "swrapper.h"

#include <iostream>

MainWindow::MainWindow(WindowAttachedData *data, QWidget *p)
	: QWidget(p)
{
	_d = data;
	_wrap = new SWrapper(_d->storage, this);

	QSplitter *split = new QSplitter(this);
	ContainerListView *clv = new ContainerListView(this);
	FieldsListView *flv = new FieldsListView(this);

	clv->getContainerModel()->setWrapper(_wrap);
	flv->getFieldsModel()->setWrapper(_wrap);

	split->setOrientation(Qt::Horizontal);
	split->addWidget(clv);
	split->addWidget(flv);

	QVBoxLayout *lay = new QVBoxLayout;
	lay->addWidget(split);
	lay->setMargin(0);
	setLayout(lay);

	connect(clv, &ContainerListView::selectionChanged, flv, [flv](QString s) {std::cout << "Container chosen: " << s.toStdString() << std::endl; flv->setContainer(s);});
}
