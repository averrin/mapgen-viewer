//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// Palette Enums
//
// Copyright(c) 2016-2017 M.J.Silk
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions :
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software.If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
// M.J.Silk
// MJSilk2@gmail.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SELBAWARD_PALETTEENUMS_HPP
#define SELBAWARD_PALETTEENUMS_HPP

#include "Common.hpp"

namespace selbaward
{
	namespace Palette
	{
		namespace Default
		{
	
enum Colors
{
	Black = 0,
	DarkBlue,
	DarkRed,
	DarkMagenta,
	DarkGreen,
	DarkCyan,
	DarkYellow,
	DarkWhite,
	Gray,
	Blue,
	Red,
	Magenta,
	Green,
	Cyan,
	Yellow,
	White
};

		} // Default
		namespace Colors2BlackWhite
		{

enum Colors
{
	Black = 0,
	White
};

		} // namespace Colors2BlackWhite
		namespace Colors2WhiteBlack
		{

enum Colors
{
	White = 0,
	Black
};

		} // namespace Colors2WhiteBlack
		namespace Colors16Cga
		{

enum Colors
{
	Black = 0,
	Blue,
	Green,
	Cyan,
	Red,
	Magenta,
	Brown,
	LightGray,
	DarkGray,
	LightBlue,
	LightGreen,
	LightCyan,
	LightRed,
	LightMagenta,
	Yellow,
	White
};

		} // namespace Colors16Cga
		namespace Colors16CgaNonIbm
		{

enum Colors
{
	Black = 0,
	DarkBlue,
	DarkGreen,
	DarkCyan,
	DarkRed,
	DarkMagenta,
	DarkYellow,
	Gray,
	Black2,
	Blue,
	Green,
	Cyan,
	Red,
	Magenta,
	Yellow,
	White
};

		} // namespace Colors16CgaNonIbm
		namespace Colors16Windows
		{

enum Colors
{
	Black = 0,
	DarkRed,
	DarkGreen,
	DarkYellow,
	DarkBlue,
	DarkMagenta,
	DarkCyan,
	DarkWhite,
	LightBlack,
	Red,
	Green,
	Yellow,
	Blue,
	Magenta,
	Cyan,
	White
};

		} // namespace Colors16Windows
		namespace Colors16Mac
		{

enum Colors
{
	White = 0,
	Yellow,
	Orange,
	Red,
	Magenta,
	Purple,
	Blue,
	Cyan,
	Green,
	DarkGreen,
	Brown,
	Tan,
	LightGray,
	MediumGray,
	DarkGray,
	Black
};

		} // namespace Colors16Mac
		namespace Colors16ZxSpectrum
		{

enum Colors
{
	Black = 0,
	DarkBlue,
	DarkRed,
	DarkMagenta,
	DarkGreen,
	DarkCyan,
	DarkYellow,
	DarkWhite,
	Black2,
	Blue,
	Red,
	Magenta,
	Green,
	Cyan,
	Yellow,
	White
};

		} // namespace Colors16ZxSpectrum
		namespace Colors16Html
		{

enum Colors
{
	Black = 0,
	Silver,
	Gray,
	White,
	Maroon,
	Red,
	Purple,
	Fuchsia,
	Green,
	Lime,
	Olive,
	Yellow,
	Navy,
	Blue,
	Teal,
	Aqua
};

		} // namespace Colors16Html
		namespace Colors216Web
		{

enum Colors
{
	x000 = 0,
	x003,
	x006,
	x009,
	x00c,
	x00f,
	x030,
	x033,
	x036,
	x039,
	x03c,
	x03f,
	x060,
	x063,
	x066,
	x069,
	x06c,
	x06f,
	x090,
	x093,
	x096,
	x099,
	x09c,
	x09f,
	x0c0,
	x0c3,
	x0c6,
	x0c9,
	x0cc,
	x0cf,
	x0f0,
	x0f3,
	x0f6,
	x0f9,
	x0fc,
	x0ff,
	x300,
	x303,
	x306,
	x309,
	x30c,
	x30f,
	x330,
	x333,
	x336,
	x339,
	x33c,
	x33f,
	x360,
	x363,
	x366,
	x369,
	x36c,
	x36f,
	x390,
	x393,
	x396,
	x399,
	x39c,
	x39f,
	x3c0,
	x3c3,
	x3c6,
	x3c9,
	x3cc,
	x3cf,
	x3f0,
	x3f3,
	x3f6,
	x3f9,
	x3fc,
	x3ff,
	x600,
	x603,
	x606,
	x609,
	x60c,
	x60f,
	x630,
	x633,
	x636,
	x639,
	x63c,
	x63f,
	x660,
	x663,
	x666,
	x669,
	x66c,
	x66f,
	x690,
	x693,
	x696,
	x699,
	x69c,
	x69f,
	x6c0,
	x6c3,
	x6c6,
	x6c9,
	x6cc,
	x6cf,
	x6f0,
	x6f3,
	x6f6,
	x6f9,
	x6fc,
	x6ff,
	x900,
	x903,
	x906,
	x909,
	x90c,
	x90f,
	x930,
	x933,
	x936,
	x939,
	x93c,
	x93f,
	x960,
	x963,
	x966,
	x969,
	x96c,
	x96f,
	x990,
	x993,
	x996,
	x999,
	x99c,
	x99f,
	x9c0,
	x9c3,
	x9c6,
	x9c9,
	x9cc,
	x9cf,
	x9f0,
	x9f3,
	x9f6,
	x9f9,
	x9fc,
	x9ff,
	xc00,
	xc03,
	xc06,
	xc09,
	xc0c,
	xc0f,
	xc30,
	xc33,
	xc36,
	xc39,
	xc3c,
	xc3f,
	xc60,
	xc63,
	xc66,
	xc69,
	xc6c,
	xc6f,
	xc90,
	xc93,
	xc96,
	xc99,
	xc9c,
	xc9f,
	xcc0,
	xcc3,
	xcc6,
	xcc9,
	xccc,
	xccf,
	xcf0,
	xcf3,
	xcf6,
	xcf9,
	xcfc,
	xcff,
	xf00,
	xf03,
	xf06,
	xf09,
	xf0c,
	xf0f,
	xf30,
	xf33,
	xf36,
	xf39,
	xf3c,
	xf3f,
	xf60,
	xf63,
	xf66,
	xf69,
	xf6c,
	xf6f,
	xf90,
	xf93,
	xf96,
	xf99,
	xf9c,
	xf9f,
	xfc0,
	xfc3,
	xfc6,
	xfc9,
	xfcc,
	xfcf,
	xff0,
	xff3,
	xff6,
	xff9,
	xffc,
	xfff
};

		} // namespace Colors216Web
	
	} // namespace Palette
} // namespace selbaward
#endif // SELBAWARD_PALETTEENUMS_HPP
