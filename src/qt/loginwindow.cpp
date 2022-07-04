#include "loginwindow.h"

#include "../filetree.h"
#include "mainwindow.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>

#include <iostream>
#include <unistd.h>

extern std::string gen_random(const int len);

LoginWindow::LoginWindow(QWidget *p)
	: QWidget(p)
{
	QVBoxLayout *mainLayout = new QVBoxLayout;
	QGridLayout *gridLayout = new QGridLayout;
	QHBoxLayout *btnLayout = new QHBoxLayout;

	QLabel *id = new QLabel(tr("Password:"), this);
	QLabel *src = new QLabel(tr("File:"), this);
	QLineEdit *passEdit = new QLineEdit;
	QLineEdit *fileEdit = new QLineEdit;
	QPushButton *fileBtn = new QPushButton(tr("Select..."));

	passEdit->setPlaceholderText(tr("my pass word"));
	fileEdit->setPlaceholderText(tr("/home/me/file.cpw"));

	gridLayout->addWidget(id, 0, 0, 1, 1);
	gridLayout->addWidget(src, 1, 0, 1, 1);
	gridLayout->addWidget(passEdit, 0, 1, 1, 4);
	gridLayout->addWidget(fileEdit, 1, 1, 1, 3);
	gridLayout->addWidget(fileBtn, 1, 4, 1, 1);

	QPushButton *cancel = new QPushButton(tr("Close"));
	QPushButton *ok = new QPushButton(tr("Open"));

	btnLayout->addWidget(cancel);
	btnLayout->addWidget(ok);

	mainLayout->addLayout(gridLayout);
	mainLayout->addLayout(btnLayout);

	setLayout(mainLayout);
	setWindowTitle(tr("Select password file."));

	connect(cancel, &QPushButton::clicked, this, [this]() {
		close();
	});
	connect(fileBtn, &QPushButton::clicked, this, [this, fileEdit]() {
		QString s = QFileDialog::getSaveFileName(this, tr("Choose a passwords file"), "://computer", tr("CPass Words file (*.cpw)"));
		if (!s.isEmpty()) {
			fileEdit->setText(s);
		}
	});
	connect(ok, &QPushButton::clicked, this, [this, passEdit, fileEdit]() {
		Document doc = loadFromFile(fileEdit->text().toStdString());
		if (doc.valid) {
			data.cypher = Cypheria();
			data.cypher.setSalt(doc.header.salt);
			data.cypher.setKey(passEdit->text().toStdString());
			data.cypher.isSame("", "");

			if (data.cypher.isSame(passEdit->text().toStdString(), doc.header.sample)) {
				data.storage = Storage();
				data.storage.setCypher(&data.cypher);
				data.storage.loadFrom(&doc);
				MainWindow *win = new MainWindow(&data);
				win->show();
				close();
			}
		} else {
			data.cypher = Cypheria();
			doc = Document();

			doc.header.salt = gen_random(16);
			data.cypher.setSalt(doc.header.salt);
			data.cypher.setKey(passEdit->text().toStdString());
			doc.header.sample = data.cypher.crypt(passEdit->text().toStdString(), &doc.header.size, &doc.header.fill);

			data.storage = Storage();
			data.storage.loadFrom(&doc);

			MainWindow *win = new MainWindow(&data);
			win->show();
			close();
		}
	});
}
