#pragma once

#include <string>
#include <sstream>
#include <raylib.h>

std::string WrapText(const std::string& text, float maxWidth, float fontSize, float spacing, Font font);

