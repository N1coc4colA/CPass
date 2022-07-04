#include "swrapper.h"

#define STR(x) x.toStdString()
#define QS(x) QString::fromStdString(x)

SWrapper::SWrapper(QObject *p) : QObject(p), Storage()
{
}

SWrapper::SWrapper(Storage &s, QObject *p) : QObject(p), Storage(s)
{
}

bool SWrapper::addContainer(std::string a)
{
	bool r = Storage::addContainer(a);
	if (r) {
		Q_EMIT added(QS(a));
	}
	return r;
}

bool SWrapper::addField(std::string a, std::string n, std::string c)
{
	bool r = Storage::addField(a, n, c);
	if (r) {
		Q_EMIT fieldAdded(QS(a), QS(n));
	}
	return r;
}

bool SWrapper::removeContainer(std::string a)
{
	bool r = Storage::removeContainer(a);
	if (r) {
		Q_EMIT removed(QS(a));
	}
	return r;
}

bool SWrapper::removeField(std::string a, std::string n)
{
	bool r = Storage::removeField(a, n);
	if (r) {
		Q_EMIT fieldRemoved(QS(a), QS(n));
	}
	return r;
}

bool SWrapper::changeContainerName(std::string p, std::string n)
{
	bool r = Storage::changeContainerName(p, n);
	if (r) {
		Q_EMIT renamed(QS(p), QS(n));
	}
	return r;
}

bool SWrapper::changeFieldName(std::string a, std::string p, std::string n)
{
	bool r = Storage::changeFieldName(a, p, n);
	if (r) {
		Q_EMIT fieldRenamed(QS(a), QS(p), QS(n));
	}
	return r;
}

bool SWrapper::changeFieldValue(std::string a, std::string n, std::string v)
{
	bool r = Storage::changeFieldValue(a, n, v);
	if (r) {
		Q_EMIT fieldChanged(QS(a), QS(n));
	}
	return r;
}

bool SWrapper::setImageValue(std::string a, std::string v)
{
	bool r = Storage::setImageValue(a, v);
	if (r) {
		Q_EMIT iconChanged(QS(a));
	}
	return r;
}

bool SWrapper::addContainer(QString a)
{
	return addContainer(STR(a));
}

bool SWrapper::addField(QString a, QString n, QString c)
{
	return addField(STR(a), STR(n), STR(c));
}

bool SWrapper::removeContainer(QString a)
{
	return removeContainer(STR(a));
}

bool SWrapper::removeField(QString a, QString n)
{
	return removeField(STR(a), STR(n));
}

bool SWrapper::changeContainerName(QString p, QString n)
{
	return changeContainerName(STR(p), STR(n));
}

bool SWrapper::changeFieldName(QString a, QString p, QString n)
{
	return changeFieldName(STR(a), STR(p), STR(n));
}

bool SWrapper::changeFieldValue(QString a, QString n, QString v)
{
	return changeFieldValue(STR(a), STR(n), STR(v));
}

bool SWrapper::setImageValue(QString a, QString v)
{
	return setImageValue(STR(a), STR(v));
}

QList<QString> SWrapper::containers()
{
	std::list<std::string> src = getContainersNames();
	QList<QString> list;
	for (auto v : src) {
		list << QS(v);
	}
	return list;
}

QList<QString> SWrapper::fields(QString a)
{
	std::list<std::string> src = getFieldsNames(STR(a));
	QList<QString> list;
	for (auto v : src) {
		list << QS(v);
	}
	return list;
}

QString SWrapper::value(QString a, QString n)
{
	return QS(Storage::value(STR(a), STR(n)));
}

QPixmap SWrapper::getPixmap(QString a)
{
	QPixmap pxm;
	pxm.loadFromData(QS(Storage::imageValue(STR(a))).toLocal8Bit());
	return pxm;
}

SWrapper *__wrap = nullptr;

std::string SWrapper::__decrypt (std::string c, size_t s, const char chr)
{
	return __wrap->cypher->decrypt(c, s, chr);
}
void SWrapper::print()
{
	Document doc = Storage::exportContent();
	__wrap = this;
	doc.print(&SWrapper::__decrypt);
}
