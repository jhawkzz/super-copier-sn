#pragma once

#include "cartPin.h"

class SNCartEnablePin : public CartPin
{
public:
	void Enable();
	void Disable();
};

class SNResetPin : public CartPin
{
public:
	void Enable();
	void Disable();
};

class SNWriteEnablePin : public CartPin
{
public:
	void Enable();
	void Disable();
};

class SNReadEnablePin : public CartPin
{
public:
	void Enable();
	void Disable();
};