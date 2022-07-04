#pragma once

#include <QWidget>

#include "windowattacheddata.h"

class LoginWindow : public QWidget
{
	Q_OBJECT
public:
	explicit LoginWindow(QWidget *p = nullptr);

	WindowAttachedData data;
};
