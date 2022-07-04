#pragma once

#include <QWidget>

class WindowAttachedData;
class SWrapper;

class MainWindow : public QWidget
{
	Q_OBJECT
public:
	explicit MainWindow(WindowAttachedData *data, QWidget *p = nullptr);

private:
	WindowAttachedData *_d = nullptr;
	SWrapper *_wrap = nullptr;
};
