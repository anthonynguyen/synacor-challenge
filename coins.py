red = 2
blue = 9
concave = 7
shiny = 5
corroded = 3

# _  + _ * _^2 + _^3 - _ = 399

import itertools
for p in itertools.permutations([red, blue, concave, shiny, corroded]):
	if p[0] + p[1] * p[2] ** 2  + p[3] ** 3 - p[4] == 399:
		print(p)
		break