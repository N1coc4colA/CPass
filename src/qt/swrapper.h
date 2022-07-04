#pragma once

#include <QObject>
#include <QPixmap>

#include "../storage.h"

class SWrapper : public QObject, public Storage
{
	Q_OBJECT
public:
	explicit SWrapper(QObject *p = nullptr);
	explicit SWrapper(Storage &s, QObject *p = nullptr);

	bool addContainer(std::string app) override;
	bool addField(std::string app, std::string name, std::string content = "") override;

	bool removeContainer(std::string app) override;
	bool removeField(std::string app, std::string name) override;

	bool changeContainerName(std::string prev, std::string next) override;
	bool changeFieldName(std::string app, std::string prev, std::string next) override;
	bool changeFieldValue(std::string app, std::string name, std::string val) override;

	bool setImageValue(std::string app, std::string value) override;


	bool addContainer(QString app);
	bool addField(QString app, QString name, QString content = "");

	bool removeContainer(QString app);
	bool removeField(QString app, QString name);

	bool changeContainerName(QString prev, QString next);
	bool changeFieldName(QString app, QString prev, QString next);
	bool changeFieldValue(QString app, QString name, QString val);

	bool setImageValue(QString app, QString value);

	QList<QString> containers();
	QList<QString> fields(QString app);
	QString value(QString app, QString name);

	QPixmap getPixmap(QString app);

	void print();

Q_SIGNALS:
	void added(QString app);
	void renamed(QString app, QString newer);
	void removed(QString app);
	void iconChanged(QString app);

	void fieldAdded(QString app, QString name);
	void fieldChanged(QString app, QString name);
	void fieldRenamed(QString app, QString name, QString newer);
	void fieldRemoved(QString app, QString name);

private:
	static std::string __decrypt(std::string c, size_t s, const char chr);
};
