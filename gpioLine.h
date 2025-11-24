#pragma once

#include <cstdint>

struct gpiod_chip;
struct gpiod_line;

class GPIOLine
{
public:
	~GPIOLine();

	void Create(gpiod_chip* pChip, uint32_t gpioLineNum);
	void HiZ();
	void Write(uint8_t bit);
	int8_t Read();
	void Release();

private:
	bool OpenLine();
	void ConfigForOutput();
	void ConfigForInput();
	void ConfigForHiZ();

private:
	int32_t mGPIOLineNum = -1;
	gpiod_line* mpLine = nullptr;
	gpiod_chip* mpChip = nullptr;

	enum class Direction
	{
		None,
		Input,
		Output,
		HiZ
	};

	Direction mDirection = Direction::None;
};
