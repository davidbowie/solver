/*
EXP-SOLVER - program calculating math expressions

solverFunc.cpp -  Functions

Copyright (C) 2015 Marcin Możejko

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <math.h>
#include "solverLib.h"

Numeric add(Numeric a, Numeric b)
{
	return a + b;
}

Numeric sub(Numeric a, Numeric b)
{
	return a - b;
}

Numeric mul(Numeric a, Numeric b)
{
	return a * b;
}

Numeric div(Numeric a, Numeric b)
{
	return a / b;
}

Numeric tg(Numeric a)
{
	return sin(a) / cos(a);
}

Numeric ctg(Numeric a)
{
	return cos(a) / sin(a);
}

Numeric sec(Numeric a)
{
	return 1 / cos(a);
}

Numeric csc(Numeric a)
{
	return 1 / sin(a);
}

Numeric atg(Numeric a)
{
	return asin(a / sqrt(1 + a * a));
}

Numeric actg(Numeric a)
{
	return acos(a / sqrt(1 + a * a));
}
